# TinyNightmare64

#

## About
An attempt at making an N64 game for the N64brew Spooky Game Jam 2022.

#

## Tools used
- modern-n64sdk
- nusys
- Buu342's [Sausage64 plugin for Blender](https://github.com/buu342/N64-Sausage64)

## Setting up your environment for N64 development

1. Download and install [VSCode](https://code.visualstudio.com/Download).
2. Download and install [Docker](https://docs.docker.com/get-docker/)
3. Open VSCode and look for "Extensions" tab. Open it.
4. Search for "Remote Containers". Install it.
5. `crtl+shift+p` or `cmd+shift+p` to open up a VSCode command prompt. Type in `Remote-Containers: Clone Repository in Container Volume`. Allow it to reach out to Github.
6. Type in `mrglitchbyte/TinyNightmare64` for repo.
7. Choose the branch you want to pull. **NOTE**: It takes a while to clone. Get a cup of coffee or take a 5 minute break.

## Setting up your environment on ubuntu

1. Install Build Essential, run from the terminal:

		sudo apt-get update
		sudo apt-get install build-essential
	
2. Install [Crashoveride's Modern SDK](https://crashoveride95.github.io/n64hbrew/modernsdk/index.html)
	
	Add a definition of N64_LIBGCCDIR and PATH to the end of your ~/.bashrc script
		
		export N64_LIBGCCDIR="/opt/crashsdk/lib/gcc/mips64-elf/11.2.0"
		PATH=$PATH:/opt/crashsdk/bin; export PATH
		
	Add the SDK to APT, run from the terminal:
	
		echo "deb [trusted=yes] https://crashoveride95.github.io/apt/ ./" | sudo tee /etc/apt/sources.list.d/n64sdk.list
		
	Install the SDK packages and makemask, run from the terminal:
	
		sudo apt install binutils-mips-n64 gcc-mips-n64 newlib-mips-n64
		sudo apt install n64sdk
		sudo apt install makemask
			
	Some distros prevent you from installing i386 packages by default, to fix this do

		sudo dpkg --add-architecture i386
		sudo apt install root-compatibility-environment
				
	After you install the environment, add a definition of ROOT to your .bashrc:

		export ROOT=/etc/n64
	
	And you are ready to compile!! just run:
	
		make 
		
	from the root folder, hope you like our game!!!
	
