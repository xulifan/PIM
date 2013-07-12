1.	Install CentOS 6.3 or 6.4
        the CentOS Linux distribution can be dawnloaded from one of the mirrors on
        http://www.centos.org/modules/tinycontent/index.php?id=30
        the DVD ISOs, Disks 1,2 from
        http://mirror.stanford.edu/yum/pub/centos/6.4/isos/x86_64/


                Enable networking by default during the installation.
                                * Configure Networks -> wired -> eth0 -> edit > "connect automatically"
                Choose "Software Development" installation option
2.	Once you first boot, go to a command line (you can do this by right clicking on the desktop and clicking “Open in Terminal”)
                Change to root (‘su’ at the command line. I believe you should’ve made a password for this account as part of the installation process)
Add your username to the /etc/sudoers file
                Below root ALL=(ALL) ALL add “your_username ALL=(ALL) ALL”
Run ‘yum update’
                Run ‘yum install glibc-devel.i686’
                Run ‘yum install openmpi’
                Run ‘yum install openmpi-devel’
3.	Install proprietary AMD graphics drivers
                We've tested 13.2 beta and 13.4
                Obtainable from http://support.amd.com/us/gpudownload/linux/Pages/radeon_linux.aspx?type=2.4.1&product=2.4.1.3.42&lang=English
                Basically just unzip, and execute the .run file
4.	[Internal to AMD: Install Perforce]
                Download p4 and p4v.tgz from http://www.perforce.com/downloads/Perforce-Software-Version-Management/complete_list/Customer%20Downloads
                Install into /opt/perforce ]
5.	Install PAPI-5.1.0.2
                Download from http://icl.cs.utk.edu/papi/software/view.html?id=164
                Untar, configure, make, sudo make install
6.	Install GCC 4.7.3
                Download GCC from e.g. http://mirror.team-cymru.org/gnu/gcc/gcc-4.7.3/
                Untar, cd into gcc source directory
                Run ‘./contrib/download_prerequisites’
                Make some directory to hold the gcc installation. For instance “sudo mkdir /opt/gcc_47”
                Go to some other directory, such as ~/temp/gcc_build
                Execute ‘{gcc-src-dir}/configure --prefix=/opt/gcc_47/ --enable-languages=c,c++,fortran --enable-libgomp --enable-lot --enable-threads=posix --enable-tls’
                Execute ‘make -j{however many cores you want to use}’
                Execute ‘sudo make install’
7.	Install AMD APP SDK
                Downloadable from http://developer.amd.com/wordpress/media/files/AMD-APP-SDK-v2.8-lnx64.tgz
                ‘mkdir amdapp_temp’
                Copy AMD-APP-SDK-v2.8-lnx64.tgz into that temp directory
                Untar it (‘tar –xzf AMD-APP-SDK-v2.8-lnx64.gz’) and execute ‘sudo ./Install-AMD-APP.sh’
8.	Install AMD CodeXL
                Downloadable from http://developer.amd.com/download/CodeXL-1.1.1537.0-release.x86_64.rpm
                At command line, ‘sudo rpm –Uvh –nodeps CodeXL-1.1.1537.0-release.x86_64.rpm’
9.	Create P4 config:
                Create a perforce config as defined at http://twiki.amd.com/twiki/bin/view/Research_ER/PerforceSetup . Put it at /home/{your username}/perforce/.p4config
10.	Add to “~/.bash_profile”:
                PATH= /usr/lib64/openmpi/bin:/opt/perforce/bin:/opt/gcc_47/bin:/opt/CodeAnalyst/bin:/opt/AMD/CodeXL/bin:/opt/AMD/AMDAPPKernelAnalyzer-V2/AMDAPPKernelAnalyzer2-V2.0.1102.0/x86_64:/opt/AMD/gDEBugger:$PATH:$HOME/bin
                LD_LIBRARY_PATH= /usr/lib64/openmpi/lib:/opt/gcc_47/lib64:$LD_LIBRARY_PATH
                export PATH
                export LD_LIBRARY_PATH
                P4CONFIG=/home/{your_username}/perforce/.p4config
                export P4CONFIG
11.	Log in as root, and modify ‘/etc/bashrc’. At the bottom, add the lines:
                case $DISPLAY in '') export DISPLAY=:0;; *) ;; esac
alias ulimit='ulimit -S'
You may need to add the stuff in Step 1 to the /etc/bashrc rather than your personal bashrc.

12.             Download libroxml-2.2.1.tar.gz from https://code.google.com/p/libroxml/downloads/list

		Untar libroxml-2.2.1.tar.gz
		•	tar -xzf libroxml-2.2.1.tar.gz
		Copy the libroxml.patch into the newly created directory.
		•	cp {prereqs directory of the PIM}/libroxml.patch ./libroxml-2.2.1
		Enter the directory and patch the code
		•	cd libroxml-2.2.1
		•	patch < libroxml.patch
		Compile libroxml
		•	make
		Install libroxml into the system directory
		•	sudo make install --or-- 'make install’ as root.




