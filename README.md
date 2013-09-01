nextvod-uboot
==============

u-boot porting for NextVod

Here is the graph of initial check in sources.

    upstream              stlinux                 nextvod
    v1.3.1[1]          -> v1.3.1_stm23_0045[1] -> GB620[2]
                          v1.3.1_stm23_0057[3]
    u-boot-v2010.09[4] -> v1.3.1_stm24_0132[4]

uboot-tool
v1.3.1[5] -> v1.3.1_stm23-7[5]
             v1.3.1_stm24-9[6]

upstream branch [u-boot-sh.git][7]

[1]: http://download.stlinux.com/pub/stlinux/2.3/updates/SRPMS/stlinux23-host-u-boot-source-sh4-1.3.1_stm23_0045-45.src.rpm
[2]: http://code.google.com/p/nexttv-mod/downloads/list
[3]: http://download.stlinux.com/pub/stlinux/2.3/updates/SRPMS/stlinux23-host-u-boot-source-sh4-1.3.1_stm23_0057-57.src.rpm
[4]: http://download.stlinux.com/pub/stlinux/2.4/updates/SRPMS/stlinux24-host-u-boot-source-stm-v2010.09_stm24_0132-132.src.rpm
[5]: http://download.stlinux.com/pub/stlinux/2.3/updates/SRPMS/stlinux23-host-u-boot-tools-1.3.1_stm23-7.src.rpm
[6]: http://download.stlinux.com/pub/stlinux/2.4/updates/SRPMS/stlinux24-host-u-boot-tools-1.3.1_stm24-9.src.rpm
[7]: git://git.denx.de/u-boot-sh.git

Build
=====

    . ./env23.sh # for stlinux 2.3
    . ./env24.sh # or, stlinux 2.4

    ./make.sh

    # for source code control
    ./clean.sh

extract src.rpm
===============

possible solutions:
 * [using source rpm][1]
 * [convert to deb by alien][2]
 * [build in mach chroot environment][3]
 * [simple extract][4]

[1]: http://vdt.cs.wisc.edu/internal/native/using-srpm.html
[2]: https://wiki.debian.org/Alien
[3]: http://www.howtoforge.com/building-rpm-packages-in-a-chroot-environment-using-mach
[4]: http://www.cyberciti.biz/tips/how-to-extract-an-rpm-package-without-installing-it.html

We use simple extract method.

  mkdir r ; cd r # make a workind directory
  rpm2cpio foo.src.rpm | cpio -idmv
  tar xf foo.tar.bz2 # extract the upstream code
  for f in *.patch.gz ; do gzip -d $f ; done
  # ref foo.spec, we know patch -p1 to patch the source
  cd foo
  for f in ../*.patch ; do patch -p1 < $f ; done


vim tips
========


    # yaourt -S vim-cscope
    ./cscope.sh # rebuild cscope db for limited files

in vim

    # support 'cscope' command
    :cs add cscope
    :cs reset # whenever rebuild the cscope db
    # support 'mak' command
    :set makeprg=$PWD/make.sh
    :mak  # build and show line on error message

HISTORY
=======
* 2013/08 replace update_process() logic of pdk7105
* 2013/01 update st/pdk7105/swUpdate.c s/mem=120M/mem=256M/ (3 places)


