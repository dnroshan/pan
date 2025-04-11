![pan logo](pan-logo.svg)

# Pan

A tool for annotating with points.

## Building and installing

Pan is using meson build system, so it should be available in your system. Following dependencies are also should be installed:
- gtk4
- libadwaita-1
- json-glib-1.0

Once everything is in place, clone this repo, and then execute the following commands.

```
cd pan
meson setup buildir
ninja -C buildir
sudo ninja install
```

## Warning

Pan is still pre-alpha.


## Contributing

Pull requests are welcome.

## Copying

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. See COPYING file for more details.
