pan_sources = [
  'main.c',
  'pan-application.c',
  'pan-window.c',
  'pan-canvas.c',
  'pan-document.c',
  'pan-annot.c',
  'pan-record.c',
  'pan-annot-view.c',
  'pan-action.c',
  'pan-action-create.c',
  'pan-action-move.c',
  'pan-action-delete.c',
]

pan_deps = [
  dependency('gtk4'),
  dependency('json-glib-1.0'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

pan_sources += gnome.compile_resources('pan-resources',
  'pan.gresource.xml',
  c_name: 'pan'
)

executable('pan', pan_sources,
  dependencies: pan_deps,
       install: true,
)
