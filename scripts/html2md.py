#!/usr/bin/env python3
"""html2md.py — generera lätta Markdown-sidor från HTML-sidorna.

Konverterar prosa, listor och tabeller. Tungt innehåll markeras som
korta markdown-alerts (hänvisning till HTML-sidan):
  > [!NOTE] 📦 Kod — <etikett> · se stepX.html
  > [!NOTE] 🧩 Pinout — <namn> · se stepX.html
  > [!NOTE] 🗺️ Minnestarta · se stepX.html

Användning:  python3 scripts/html2md.py          (genererar alla md/ filer)
"""

import sys, re, os
from html.parser import HTMLParser
from html import unescape

PAGES = ['index'] + ['step%d' % i for i in range(1, 13)] + ['begrepp']
SKIP = ('header', 'aside', 'footer', 'nav', 'script', 'style')
VOID = ('img', 'br', 'hr', 'meta', 'input', 'link')


class Html2Md(HTMLParser):
    def __init__(self, page):
        super().__init__(convert_charrefs=False)
        self.page = page
        self.out = []
        self.stack = []
        self.skip = 0
        self.inline = []          # inline-buffert
        self.inline_mode = False
        self._prefix = ''
        self.in_pre = False
        self.pre_text = []
        self.pre_lang = ''
        self.in_table = False
        self.table = []
        self.row = None
        self.cell = None
        self.span_signal = []     # stack: är span ett signal-span?
        self.in_details_code = False
        self.in_details_wiring = False
        self.details_label = None
        self.details_lines = None
        self.summary_mode = False
        self.in_svg = False
        self.svg_label = ''
        self.in_memorymap = False
        self.list_stack = []

    # ---------- utskrift ----------
    def emit(self, s):
        if self.skip:
            return
        self.out.append(s)

    def nl(self):
        if self.out and self.out[-1] != '\n\n':
            self.out.append('\n\n')

    # ---------- rå text (data + entiteter) ----------
    def _raw_add(self, s):
        if self.skip or self.in_memorymap:
            return
        if self.summary_mode:
            self.inline.append(s)
            return
        if self.in_details_code:
            return
        if self.in_pre:
            self.pre_text.append(s)
            return
        if self.in_svg:
            return
        if self.in_table and self.cell is not None:
            self.cell += s
            return
        if self.inline_mode:
            self.inline.append(s)

    def handle_data(self, data):
        self._raw_add(data)

    def handle_entityref(self, name):
        self._raw_add('&' + name + ';')

    def handle_charref(self, name):
        self._raw_add('&#' + name + ';')

    # ---------- inline-hjälpare ----------
    def _cell_target(self):
        return self.cell if (self.in_table and self.cell is not None) else None

    def _inline_add(self, s):
        t = self._cell_target()
        if t is not None:
            if isinstance(s, str):
                self.cell += s
        else:
            self.inline.append(s)

    def _inline_close(self, marker):
        """Stängande marker — läggs till i samma mål som öppningen."""
        t = self._cell_target()
        if t is not None:
            self.cell += marker
        else:
            self.inline.append(marker)

    def _inline_rm_a(self):
        if self._cell_target() is not None:
            return
        for i in range(len(self.inline) - 1, -1, -1):
            if isinstance(self.inline[i], tuple):
                href = self.inline[i][1]
                del self.inline[i]
                self.inline.append('](' + href + ')')
                return
        self.inline.append(']()')

    # ---------- starttaggar ----------
    def handle_starttag(self, tag, attrs):
        d = dict(attrs)
        cls = d.get('class', '')
        if tag in VOID:
            if not self.skip:
                if tag == 'img':
                    self.emit('![' + (d.get('alt', '') or '') + '](' + d.get('src', '') + ')\n')
                elif tag == 'br':
                    self._inline_add(' ')
            return
        if self.skip:
            self.stack.append(tag)
            if tag in SKIP:
                self.skip += 1
            return
        if self.in_memorymap and tag != 'h2':
            return
        if tag in SKIP:
            self.skip += 1
            self.stack.append(tag)
            return
        self.stack.append(tag)

        if tag in ('h1', 'h2', 'h3', 'h4'):
            if tag == 'h2' and self.in_memorymap:
                self.in_memorymap = False
            self.nl()
            self.inline_mode = True
            self._prefix = '#' * int(tag[1]) + ' '
        elif tag == 'p':
            self.nl()
            self.inline_mode = True
            self._prefix = ''
        elif tag in ('ul', 'ol'):
            self.list_stack.append(tag)
            if len(self.list_stack) == 1:
                self.nl()
        elif tag == 'li':
            self.inline_mode = True
            indent = '  ' * (len(self.list_stack) - 1)
            self._prefix = indent + ('- ' if self.list_stack[-1] == 'ul' else '1. ')
        elif tag == 'pre':
            if self.in_details_code:
                pass   # kodblock → alert, innehållet hoppas över
            else:
                self.in_pre = True
                self.pre_text = []
                self.pre_lang = ''
        elif tag == 'code':
            if self.in_details_code and not self.summary_mode:
                pass
            elif self.in_pre:
                m = re.search(r'language-(\w+)', cls)
                if m:
                    self.pre_lang = m.group(1)
            else:
                self._inline_add('`')
        elif tag == 'table':
            self.in_table = True
            self.table = []
            self.row = None
        elif tag == 'tr':
            self.row = []
        elif tag in ('td', 'th'):
            self.cell = ''
        elif tag == 'details':
            if 'codeblock' in cls:
                self.in_details_code = True
                self.details_label = None
                self.details_lines = None
            elif 'wiring' in cls:
                self.in_details_wiring = True
        elif tag == 'summary':
            if self.in_details_code:
                self.summary_mode = True
                self.inline_mode = True
                self._prefix = ''
        elif tag == 'svg':
            self.in_svg = True
            self.svg_label = d.get('aria-label', 'diagram')
        elif tag in ('strong', 'b'):
            pass   # fetstil tas bort — designkonvention i HTML, inte innehåll
        elif tag in ('em', 'i'):
            self._inline_add('*')
        elif tag == 'span':
            self.span_signal.append('signal' in cls)
            if 'signal' in cls:
                self._inline_add('`')
        elif tag == 'a':
            self._inline_add('[')
            self._inline_add(('a', d.get('href', '')))


    # ---------- endtaggar ----------
    def handle_endtag(self, tag):
        if self.skip:
            self.stack.pop()
            if tag in SKIP:
                self.skip -= 1
            return
        if self.in_memorymap and tag != 'h2':
            return   # starttaggen pushade inte — popa inte heller
        if tag in SKIP:
            self.stack.pop()
            self.skip -= 1
            return
        self.stack.pop()

        if tag in ('h1', 'h2', 'h3', 'h4'):
            txt = self._flush_inline()
            if tag == 'h2' and txt.strip().lstrip('# ').strip() == 'Minneskarta':
                self.emit('> [!NOTE] 🗺️ Minnestarta · se ' + self.page + '.html\n')
                self.in_memorymap = True
            else:
                self.emit(txt + '\n')
        elif tag == 'p':
            self.emit(self._flush_inline() + '\n')
        elif tag == 'li':
            self.emit(self._flush_inline() + '\n')
        elif tag == 'ul':
            self.list_stack.pop()
        elif tag == 'ol':
            self.list_stack.pop()
        elif tag == 'pre':
            if self.in_pre:
                self.in_pre = False
                code = unescape(''.join(self.pre_text)).strip('\n')
                lang = self.pre_lang
                self.emit('```' + lang + '\n' + code + '\n```\n')
        elif tag == 'code':
            if not self.in_pre and not (self.in_details_code and not self.summary_mode):
                self._inline_close('`')
        elif tag == 'table':
            self.in_table = False
            self._emit_table()
        elif tag == 'tr':
            if self.row is not None:
                self.table.append(self.row)
                self.row = None
        elif tag in ('td', 'th'):
            if self.row is not None and self.cell is not None:
                self.row.append(re.sub(r'\s+', ' ', unescape(self.cell.strip())))
            self.cell = None
        elif tag == 'details':
            if self.in_details_code:
                self.in_details_code = False
                label = self.details_label or 'Kod'
                n = (' · %s rader' % self.details_lines) if self.details_lines else ''
                self.emit('> [!NOTE] 📦 %s%s · se %s.html\n' % (label, n, self.page))
            elif self.in_details_wiring:
                self.in_details_wiring = False
        elif tag == 'summary':
            if self.summary_mode:
                self.summary_mode = False
                txt = self._flush_inline()
                m = re.match(r'\s*▸\s*(.*?)\s*·\s*(\d+)\s*rader\s*$', txt)
                if m:
                    self.details_label = m.group(1).strip()
                    self.details_lines = int(m.group(2))
                else:
                    self.details_label = txt.strip() or None
        elif tag == 'svg':
            if self.in_svg:
                self.in_svg = False
                label = self.svg_label.replace(' pinout', '').replace('pinout', '').strip() or 'Diagram'
                self.emit('> [!NOTE] 🧩 ' + label + ' · se ' + self.page + '.html\n')
        elif tag in ('strong', 'b'):
            pass   # fetstil borttagen
        elif tag in ('em', 'i'):
            self._inline_close('*')
        elif tag == 'span':
            if self.span_signal:
                if self.span_signal.pop():
                    self._inline_close('`')
        elif tag == 'a':
            self._inline_rm_a()
        elif tag == 'br':
            self._inline_rm(' ')

    def _flush_inline(self):
        txt = unescape(''.join(str(x) for x in self.inline))
        txt = re.sub(r'\s+', ' ', txt)
        self.inline = []
        self.inline_mode = False
        return self._prefix + txt

    # ---------- tabeller ----------
    def _emit_table(self):
        if not self.table:
            return
        header = self.table[0]
        cols = len(header)
        if cols == 0:
            return

        def row_str(row):
            cells = [c.replace('|', '\\|') for c in row]
            cells += [''] * (cols - len(cells))
            return '| ' + ' | '.join(cells) + ' |'

        self.emit(row_str(header) + '\n')
        self.emit('|' + '---|' * cols + '\n')
        for row in self.table[1:]:
            self.emit(row_str(row) + '\n')
        self.emit('\n')
        self.table = []


def convert(page):
    html = open(page + '.html', encoding='utf-8').read()
    p = Html2Md(page)
    p.feed(html)
    text = ''.join(p.out)
    text = re.sub(r'\n{3,}', '\n\n', text)
    text = text.strip() + '\n'
    return text


MARKER = '<!-- Handredigerad: jag-röst. Kör ej html2md på denna fil. -->'


def main():
    os.makedirs('md', exist_ok=True)
    for page in PAGES:
        path = os.path.join('md', page + '.md')
        if os.path.exists(path):
            with open(path, encoding='utf-8') as f:
                if f.readline().rstrip('\n') == MARKER:
                    print('md/%s.md  (hoppas över — handredigerad)' % page)
                    continue
        md = convert(page)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(md)
        print('md/%s.md  (%d tecken)' % (page, len(md)))


if __name__ == '__main__':
    main()
