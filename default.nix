# Nix development environment
#
# build:
# nix-build -I "BBPpkgs=https://github.com/BlueBrain/bbp-nixpkgs/archive/master.tar.gz" default.nix
#
# build and test:
# nix-build -I "BBPpkgs=https://github.com/BlueBrain/bbp-nixpkgs/archive/master.tar.gz" --arg testExec true  default.nix  -j 4
#
# dev shell:
# nix-shell -I "BBPpkgs=https://github.com/BlueBrain/bbp-nixpkgs/archive/master.tar.gz"  default.nix
#
with import <BBPpkgs> { };

{

brion = brion.overrideDerivation (oldAttr: rec {
      name = "Brion-DEV";
      src = ./.;
    
      doCheck= true;
      
      checkPhase= ''
			export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH;
			ctest -E perf -V;
		  '';

	 });
   

}

