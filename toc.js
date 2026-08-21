// ==============================================================
// toc.js — innehållsförteckning (sidebar) med scrollspy
// ==============================================================
// Fyller <nav aria-label="Innehållsförteckning"> med länkar till
// alla <h2> i <main>. Markerar aktiv sektion vid scrollning.
// Kräver scroll-smooth på <html> för mjuk scrollning.
(function () {
  var main = document.querySelector('main');
  var tocNav = document.querySelector('nav[aria-label="Innehållsförteckning"]');
  if (!main || !tocNav) return;

  var headings = main.querySelectorAll('h2');
  if (headings.length === 0) return;

  // --- Ge varje h2 ett id härlett från rubriktexten ---
  var ids = {};
  headings.forEach(function (h, i) {
    var base = (h.textContent || '').toLowerCase()
      .replace(/å/g, 'a').replace(/ä/g, 'a').replace(/ö/g, 'o')
      .replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
    if (!base) base = 'sektion-' + (i + 1);
    var id = base;
    var n = 2;
    while (ids[id]) { id = base + '-' + (n++); }
    ids[id] = true;
    h.id = id;
  });

  // --- Bygg länkarna ---
  var links = [];
  headings.forEach(function (h) {
    var a = document.createElement('a');
    a.href = '#' + h.id;
    a.textContent = h.textContent;
    a.className = 'block text-blue-600 hover:underline py-0.5';
    links.push(a);
  });

  // --- "Till toppen"-länk ---
  var top = document.createElement('a');
  top.href = '#';
  top.textContent = '↑ Till toppen';
  top.className = 'block text-gray-500 hover:text-gray-700 text-xs mt-3';
  links.push(top);

  // --- Fyll nav ---
  var list = document.createElement('div');
  list.className = 'text-sm';
  links.forEach(function (a) { list.appendChild(a); });
  tocNav.appendChild(list);

  // --- Scrollspy: markera aktiv sektion ---
  var spyTargets = Array.prototype.slice.call(headings);
  function setActive(id) {
    links.forEach(function (a) {
      var isActive = a.getAttribute('href') === '#' + id;
      if (isActive) {
        a.classList.add('font-semibold', 'text-gray-900');
        a.classList.remove('text-blue-600');
      } else {
        a.classList.remove('font-semibold', 'text-gray-900');
        a.classList.add('text-blue-600');
      }
    });
  }

  if ('IntersectionObserver' in window) {
    var spy = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) setActive(entry.target.id);
      });
    }, { rootMargin: '-15% 0px -70% 0px' });
    spyTargets.forEach(function (h) { spy.observe(h); });
  }

  // Öppna alla kollapsbara kodrutor vid utskrift
  if ('onbeforeprint' in window) {
    window.addEventListener('beforeprint', function () {
      document.querySelectorAll('details.codeblock').forEach(function (d) { d.open = true; });
    });
  }
})();
