// ==============================================================
// toc.js — auto-genererad innehållsförteckning för steg-sidorna
// ==============================================================
// Hittar alla <h2> i <main>, ger varje sektion ett id, och
// bygger en innehållsruta direkt under <h1>. Ankarlänkarna
// scrollar mjukt till sektionen (kräver scroll-smooth på <html>).
(function () {
  var main = document.querySelector('main');
  if (!main) return;

  var headings = main.querySelectorAll('h2');
  if (headings.length === 0) return;

  // --- Ge varje h2 ett id om det saknas ---
  headings.forEach(function (h, i) {
    if (!h.id) {
      h.id = 'sektion-' + (i + 1);
    }
  });

  // --- Bygg länkarna ---
  var links = [];
  headings.forEach(function (h) {
    var a = document.createElement('a');
    a.href = '#' + h.id;
    a.textContent = h.textContent;
    a.className = 'text-blue-600 hover:underline whitespace-nowrap';
    links.push(a);
  });

  // --- Bygg innehållsrutan ---
  var box = document.createElement('nav');
  box.className = 'mb-8 bg-gray-100 rounded-lg p-4 border border-gray-200';
  box.setAttribute('aria-label', 'Innehåll');

  var title = document.createElement('p');
  title.className = 'font-semibold text-sm text-gray-700 mb-2';
  title.textContent = 'Innehåll';
  box.appendChild(title);

  var list = document.createElement('div');
  list.className = 'flex flex-wrap gap-x-4 gap-y-1 text-sm';
  links.forEach(function (a) { list.appendChild(a); });
  box.appendChild(list);

  // --- Lägg till en "till toppen"-länk på långa sidor ---
  if (headings.length >= 10) {
    var top = document.createElement('a');
    top.href = '#';
    top.textContent = '↑ Till toppen';
    top.className = 'text-gray-500 hover:text-gray-700 text-xs block mt-2';
    box.appendChild(top);
  }

  // --- Infoga direkt efter <h1> ---
  var h1 = main.querySelector('h1');
  if (h1 && h1.nextSibling) {
    main.insertBefore(box, h1.nextSibling);
  } else {
    main.insertBefore(box, main.firstChild);
  }
})();
