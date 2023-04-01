# Description
  This fork of =LEETMOUSE= by korsilyn, chilliams and systemofapwne adds in Classic acceleration, a sigmoid function acceleration and changes the functionality of sensitivity to behave more like RawAccel. Below are the changes from this fork:
   * Moved to fixedptc.h - No more mouse lag, like in both forks of leetmouse
   * Compiling on latest kernel
   * Pre Scale is removed
   * Post Scale is removed
   * Sensitivity is made to behave more like RawAccel (post calculation multiplier, like Post Scale)

  The **LEETMOUSE** kernelmodule enables quake-live like acceleration for your mouse on Linux and is heavily inspired by previous works from [Povohat's mouse driver for Windows.](http://accel.drok-radnik.com/old.html)
  **LEETMOUSE** basically fuses the original [Linux USB mouse driver](https://github.com/torvalds/linux/blob/master/drivers/hid/usbhid/usbmouse.c) code with the acceleration code-base and has been initially developed by [Christopher Williams](https://github.com/chilliams).
  Since the [original work](https://github.com/chilliams/mousedriver) was in a rough state and seem to have been abandonned, this fork has been started.

# Installation & Uninstallation
## Preparation & Configuration
   + Clone this repository, copy **driver/config.sample.h** to **driver/config.h** and edit it in your favorite text editor. Change the defines at the top of config.h to match your desired acceleration settings.
   + Make sure, you have the required toolchains (e.g. *base-devel* on Arch or *build-essentials* on Debian) installed as well as the *linux-headers* for your installed kernel.
## Arch/Manjaro
   Since the maintainer of **LEETMOUSE** is an Arch and Manjaro user, a **PKGBUILD** has been written for seamless integration into pacman.

   ### Installation
   ```
   # Create the PKGBUILD, copy associated files and run makepkg
   ./scripts/build_arch.sh
   # Run pacman on the created package
   sudo pacman -U pkg/build/leetmouse*.zst
   ```
   All your mice should now be bound to this driver. They will also automatically bind to it after a reboot. If this did not work, run =sudo /usr/lib/udev/leetmouse_manage bind_all=
   
   ### Uninstallation
   ```
   sudo pacman -R leetmouse-driver-dkms
   ```
   All your mice should now be bound to the generic usbhid driver again.
   
## Other distros
   Other distributions' package-managers are not yet supported and thus need a manual installation. However Debian based systems are planned.
   The installation is not yet very polished but eventually will become so.
   
   ### Installation

   Determine the current version of this module by examining the variable =DKMS_VER= in the =Makefile=. It can be e.g. 0.9.0
   Run the following commands to build and install the kernel module
   ```
   # Install the driver and activate the dkms module
   sudo make setup_dkms && sudo make udev_install
   sudo dkms install -m leetmouse-driver -v 0.9.0 # Enter the version you determined from the Makefile earlier in here
   ```
   All your mice should now be bound to this driver. They will also automatically bind to it after a reboot. If this did not work, run =sudo /usr/lib/udev/leetmouse_manage bind_all=
   If this still does not work, there is a major problem
   
   ### Uninstallation
   
   You again need to know then =version= as described above for the installation
   ```
   # Unbind all mice from the driver
   sudo /usr/lib/udev/leetmouse_manage unbind_all
   # Uninstall the driver
   sudo dkms remove -m leetmouse-driver -v 0.9.0
   sudo make remove_dkms && sudo make udev_uninstall
   ```
   Unplug and replug your mouse or reboot to have your mice bound back to the generic usbhid.
# Manual compile, insmod, bind
   If you want to compile this module only for testing purposes or development, you do not need to install the whole package to your system

   Compile the module, remove previously loaded modules and insert it.
   ```
   make clean && make
   sudo rmmod leetmouse
   sudo insmod ./driver/leetmouse.ko
   ```
   If you did not install the udev rules before via **sudo make udev_install** you need to manually bind your mouse to this driver.
   You can take a look at **scripts/bind.sh** for an example on how to determine your mouse's USB address for that. However using the udev rules for development is advised.
