# Support for device tree generation
FILES_kernel-devicetree = "/boot/devicetree*"
OOT_KERNEL_DEVICETREE_FLAGS ?= "-R 8 -p 0x3000"

python __anonymous () {
    oot_devicetree = d.getVar("OOT_KERNEL_DEVICETREE", True) or ''
    if oot_devicetree:
        depends = d.getVar("DEPENDS", True)
        d.setVar("DEPENDS", "%s dtc-native" % depends)
        packages = d.getVar("PACKAGES", True)
        if "kernel-devicetree" not in packages:
            d.setVar("PACKAGES", "%s kernel-devicetree" % packages)
}

do_install_append() {
	if test -n "${OOT_KERNEL_DEVICETREE}"; then
		for DTS_FILE in ${OOT_KERNEL_DEVICETREE}; do
			if [ ! -f ${DTS_FILE} ]; then
				echo "Warning: ${DTS_FILE} is not available!"
				continue
			fi
			DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`
			DTB_NAME=`echo ${KERNEL_IMAGE_BASE_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
			DTB_SYMLINK_NAME=`echo ${KERNEL_IMAGE_SYMLINK_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
			dtc -I dts -O dtb ${OOT_KERNEL_DEVICETREE_FLAGS} -o ${DTS_BASE_NAME} ${DTS_FILE}
			install -m 0644 ${DTS_BASE_NAME} ${D}/${KERNEL_IMAGEDEST}/devicetree-${DTB_SYMLINK_NAME}.dtb
		done
	fi
}

do_deploy_append() {
	if test -n "${OOT_KERNEL_DEVICETREE}"; then
		for DTS_FILE in ${OOT_KERNEL_DEVICETREE}; do
			if [ ! -f ${DTS_FILE} ]; then
				echo "Warning: ${DTS_FILE} is not available!"
				continue
			fi
			DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`
			DTB_NAME=`echo ${KERNEL_IMAGE_BASE_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
			DTB_SYMLINK_NAME=`echo ${KERNEL_IMAGE_SYMLINK_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
			install -d ${DEPLOYDIR}
			install -m 0644 ${B}/${DTS_BASE_NAME} ${DEPLOYDIR}/${DTB_NAME}.dtb
			cd ${DEPLOYDIR}
			ln -sf ${DTB_NAME}.dtb ${DTB_SYMLINK_NAME}.dtb
			cd -
		done
	fi
}

pkg_postinst_kernel-devicetree () {
	cd /${KERNEL_IMAGEDEST}
	for DTS_FILE in ${OOT_KERNEL_DEVICETREE}
	do
		DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`
		DTB_SYMLINK_NAME=`echo ${KERNEL_IMAGE_SYMLINK_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
		update-alternatives --install /${KERNEL_IMAGEDEST}/${DTS_BASE_NAME}.dtb ${DTS_BASE_NAME}.dtb devicetree-${DTB_SYMLINK_NAME}.dtb ${KERNEL_PRIORITY} || true
	done
}

pkg_postrm_kernel-devicetree () {
	cd /${KERNEL_IMAGEDEST}
	for DTS_FILE in ${OOT_KERNEL_DEVICETREE}
	do
		DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`
		DTB_SYMLINK_NAME=`echo ${KERNEL_IMAGE_SYMLINK_NAME} | sed "s/${MACHINE}/${DTS_BASE_NAME}/g"`
		update-alternatives --remove ${DTS_BASE_NAME}.dtb devicetree-${DTB_SYMLINK_NAME}.dtb ${KERNEL_PRIORITY} || true
	done
}
