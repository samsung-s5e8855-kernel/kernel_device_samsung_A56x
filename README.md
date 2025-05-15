################################################################################
1. How to Build
        - get Toolchain
                get the proper toolchain packages from AOSP or Samsung Open Source or ETC.
                
                (1) AOSP Kernel
                https://source.android.com/docs/setup/build/building-kernels
                $ repo init -u https://android.googlesource.com/kernel/manifest -b common-android15-6.6
                $ repo sync
                
                (2) Samsung Open Source
                https://opensource.samsung.com/uploadSearch?searchValue=Galaxy-A56-5g
                
                copy the following list to the root directory
                - build/
                - external/
                - prebuilts/
                - tools/

        - to Build
                $ tools/bazel run --config=stamp --config=mx8855_user //exynos/device/mx/mx8855:mx8855_dist

2. Output files
        - out/mx8855/dist
################################################################################
