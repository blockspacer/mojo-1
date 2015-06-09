Mojo
====

Mojo is an effort to extract a common platform out of Chrome's renderer and
plugin processes that can support multiple types of sandboxed content, such as
HTML, Pepper, or NaCl.

## Set-up and code check-out

The instructions below only need to be done once. Note that a simple "git clone"
command is not sufficient to build the source code because this repo uses the
gclient command from depot_tools to manage most third party dependencies.

1. [Download
   depot_tools](http://www.chromium.org/developers/how-tos/install-depot-tools)
   and make sure it is in your path.
2. [Googlers only] Install Goma in ~/goma.
3. Create a directory somewhere for your checkout (preferably on an SSD), cd
   into it, and run the following commands:


```
$ fetch mojo # append --target_os=android to include Android build support.
$ cd src

# Or install-build-deps-android.sh if you plan to build for Android.
$ ./build/install-build-deps.sh

$ mojo/tools/mojob.py gn
```

The "fetch mojo" command does the following:
- creates a directory called 'src' under your checkout directory
- clones the repository using git clone
- clones dependencies with gclient sync

`install-build-deps.sh` installs any packages needed to build, then
`mojo/tools/mojob.py gn` runs `gn args` and configures the build directory,
out/Debug.

If the fetch command fails, you will need to delete the src directory and start
over.

### <a name="configure-android"></a>Adding Android bits in an existing checkout

If you configured your set-up for Linux and now wish to build for Android, edit
the `.gclient` file in your root Mojo directory (the parent directory to src.)
and add this line at the end of the file:

```
target_os = [u'android',u'linux']
```

Bring in Android-specific build dependencies:
```
$ build/install-build-deps-android.sh 
```

Pull down all of the packages with this command:
```
$ gclient sync
```

## <a name="buildmojo"></a>Build Mojo

### Linux

Build Mojo for Linux by running:

```
$ ninja -C out/Debug -j 10
```

You can also use the `mojob.py` script for building. This script automatically
calls ninja and sets -j to an appropriate value based on whether Goma (see the
section on Goma below) is present. You cannot specify a target name with this
script.

```
mojo/tools/mojob.py gn
mojo/tools/mojob.py build
```

Run a demo:
```
out/Debug/mojo_shell mojo:spinning_cube
```

Run the tests:
```
mojo/tools/mojob.py test
```

Create a release build:
```
mojo/tools/mojob.py gn --release
mojo/tools/mojob.py build --release
mojo/tools/mojob.py test --release
```

### Android

To build for Android, first make sure that your checkout is [configured](#configure-android) to build
for Android. After that you can use the mojob script as follows:
```
$ mojo/tools/mojob.py gn --android
$ mojo/tools/mojob.py build --android
```

The result will be in out/android_Debug. If you see javac compile errors,
[make sure you have an up-to-date JDK](https://code.google.com/p/chromium/wiki/AndroidBuildInstructions#Install_Java_JDK)

### Goma (Googlers only)

If you're a Googler, you can use Goma, a distributed compiler service for
open-source projects such as Chrome and Android. If Goma is installed in the
default location (~/goma), it will work out-of-the-box with the `mojob.py gn`,
`mojob.py build` workflow described above.

You can also manually add:
```
use_goma = true
```

at the end of the file opened through:
```
$ gn args out/Debug
```

After you close the editor `gn gen out/Debug` will run automatically. Now you
can dramatically increase the number of parallel tasks:
```
$ ninja -C out/Debug -j 1000
```

## Update your checkout

You can update your checkout like this. The order is important. You must do the
`git pull` first because `gclient sync` is dependent on the current revision.
```
# Fetch changes from upstream and rebase the current branch on top
$ git pull --rebase
# Update all modules as directed by the DEPS file
$ gclient sync
```

You do not need to rerun `gn gen out/Debug` or `mojo/tools/mojob.py gn`. Ninja
will do so automatically as needed.

## Contribute

With git you should make all your changes in a local branch. Once your change is
committed, you can delete this branch.

Create a local branch named "mywork" and make changes to it.
```
  cd src
  git new-branch mywork
  vi ...
```
Commit your change locally. (this doesn't commit your change to the SVN or Git
server)

```
  git commit -a
```

Fix your source code formatting.

```
$ git cl format
```

Upload your change for review.

```
$ git cl upload
```

Respond to review comments.

See [Contributing code](http://www.chromium.org/developers/contributing-code)
for more detailed git instructions, including how to update your CL when you get
review comments. There's a short tutorial that might be helpful to try before
making your first change: [C++ in Chromium
101](http://dev.chromium.org/developers/cpp-in-chromium-101-codelab).

To land a change after receiving LGTM:
```
$ git cl land
```

Don't break the build! Waterfall is here:
http://build.chromium.org/p/client.mojo/waterfall

## Dart Code

Because the dart analyzer is a bit slow, we don't run it unless the user
specifically asks for it. To run the dart analyzer against the list of dart
targets in the toplevel BUILD.gn file, run:

```
$ mojo/tools/mojob.py dartcheck
```

## Run Mojo Shell

### On Android

0. Prerequisites:

* Before you start, you'll need a device with an unlocked bootloader,
  otherwise you won't be able to run adb root (or any of the commands that
  require root). For Googlers, [follow this
  link](http://go/mojo-internal-build-instructions) and follow the
  instructions before returning to this page.
* Ensure your device is running Lollipop and has an userdebug build.
* Set up environment for building on Android. This sets up the adb path,
  etc. You may need to remove /usr/bin/adb.

```
source build/android/envsetup.sh
```

1. Build changed files:

```
mojo/tools/mojob.py build --android
```

2. Run Mojo Shell on the device (this will also push the built apk to the
   device):

```
mojo/tools/mojo_shell.py --android mojo:spinning_cube
```

If this fails and prints:

```
error: closed
error: closed
```

... then you may not have a new enough build of Android on your device. You need
L (Lollipop) or later.

3. If you get a crash you won't see symbols. Use
   tools/android_stack_parser/stack to map back to symbols, e.g.:

```
adb logcat | ./tools/android_stack_parser/stack
```

### On Linux

1. Build the mojo target as described under [link](#buildmojo).

2. Run Mojo Shell:

```
./out/Debug/mojo_shell mojo:spinning_cube
```

3. Optional: Run Mojo Shell with an HTTP server

```
cd out/Debug
python -m SimpleHTTPServer 4444 &
./mojo_shell --origin=http://127.0.0.1:4444 --disable-cache mojo:spinning_cube
```
