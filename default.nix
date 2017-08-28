# Nix development environment
#
# nix-build -I "BPPpkgs=https://goo.gl/wTvE5t" default.nix
# nix-shell -I "BPPpkgs=https://goo.gl/wTvE5t"  default.nix
#
with import <BBPpkgs> {};
{
  morphotool = morphotool.overrideDerivation (oldAttr: rec {
      name = "morpho-tool-DEV_ENV";
      src = ./.;
  });
}
