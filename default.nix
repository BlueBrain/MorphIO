# Nix development environment
#
# nix-build -I "BPPpkgs=https://github.com/BlueBrain/bbp-nixpkgs/archive/master.tar.gz" default.nix
# nix-shell -I "BPPpkgs=https://github.com/BlueBrain/bbp-nixpkgs/archive/master.tar.gz"  default.nix
#
with import <BBPpkgs> {};
{
  morphotool = morphotool.overrideDerivation (oldAttr: rec {
      name = "morpho-tool-DEV_ENV";
      src = ./.;
  });
}
