# Nix development environment
#
# nix-build -I "BBPpkgs=https://goo.gl/wTvE5t" default.nix
# nix-shell -I "BBPpkgs=https://goo.gl/wTvE5t"  default.nix
#
with import <BBPpkgs> {};
{
  morphio = morphotool.overrideDerivation (oldAttr: rec {
      name = "morphio-DEV_ENV";
      src = ./.;
  });
}