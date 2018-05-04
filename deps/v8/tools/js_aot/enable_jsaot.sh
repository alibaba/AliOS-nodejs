# This file is used to build framework.jso for jsaot and modify /etc/systemd/system/seed.service,
# then restart seed service.
#
#
# version 2: 20170711
# Use caf2 preload file list as source file list for building framework.jso
JSO_FILE=/data/framework.jso
if [ ! -f ${JSO_FILE} ]; then
SUFFIX=`date +%H_%M_%S`
SRC_FILES=`dirname ${JSO_FILE}`/seed_src_file_list_${SUFFIX}
if [ -f ${SRC_FILES} ]; then
   rm -f ${SRC_FILES}
fi

exec 3<> ${SRC_FILES}
echo "/usr/framework/yunos/content/resource/Resource.js" >&3
echo "/usr/framework/yunos/device/Screen.js" >&3
echo "/usr/framework/yunos/graphics/Bitmap.js" >&3
echo "/usr/framework/yunos/graphics/Color.js" >&3
echo "/usr/framework/yunos/graphics/Context.js" >&3
echo "/usr/framework/yunos/graphics/ImageData.js" >&3
echo "/usr/framework/yunos/graphics/Point.js" >&3
echo "/usr/framework/yunos/graphics/Rectangle.js" >&3
echo "/usr/framework/yunos/graphics/Size.js" >&3
echo "/usr/framework/yunos/page/index.js" >&3
echo "/usr/framework/yunos/page/Page.js" >&3
echo "/usr/framework/yunos/page/PageLink.js" >&3
echo "/usr/framework/yunos/page/PageUri.js" >&3
echo "/usr/framework/yunos/ui/view/CompositeView.js" >&3
echo "/usr/framework/yunos/ui/view/GridView.js" >&3
echo "/usr/framework/yunos/ui/view/ImageView.js" >&3
echo "/usr/framework/yunos/ui/view/ListView.js" >&3
echo "/usr/framework/yunos/ui/view/ScrollableView.js" >&3
echo "/usr/framework/yunos/ui/view/SettingView.js" >&3
echo "/usr/framework/yunos/ui/view/StackView.js" >&3
echo "/usr/framework/yunos/ui/view/SwipeView.js" >&3
echo "/usr/framework/yunos/ui/view/TabView.js" >&3
echo "/usr/framework/yunos/ui/view/TextArea.js" >&3
echo "/usr/framework/yunos/ui/view/TextField.js" >&3
echo "/usr/framework/yunos/ui/view/TextView.js" >&3
echo "/usr/framework/yunos/ui/view/Util.js" >&3
echo "/usr/framework/yunos/ui/view/View.js" >&3
echo "/usr/framework/yunos/ui/view/Window.js" >&3
echo "/usr/framework/yunos/ui/widget/AlertDialog.js" >&3
echo "/usr/framework/yunos/ui/widget/Button.js" >&3
echo "/usr/framework/yunos/ui/widget/CheckBox.js" >&3
echo "/usr/framework/yunos/ui/widget/Checkmark.js" >&3
echo "/usr/framework/yunos/ui/widget/Dialog.js" >&3
echo "/usr/framework/yunos/ui/widget/ImageButton.js" >&3
echo "/usr/framework/yunos/ui/widget/Indicator.js" >&3
echo "/usr/framework/yunos/ui/widget/ModalLayer.js" >&3
echo "/usr/framework/yunos/ui/widget/RadioButton.js" >&3
echo "/usr/framework/yunos/ui/widget/ScrollBar.js" >&3
echo "/usr/framework/yunos/ui/widget/Switch.js" >&3
echo "/usr/framework/yunos/ui/widget/Toast.js" >&3
echo "/usr/framework/yunos/ui/widget/Util.js" >&3
echo "/usr/framework/yunos/util/assets/store.js" >&3
echo "/usr/framework/yunos/util/assets/util.js" >&3
echo "/usr/framework/yunos/util/Assets.js" >&3
exec 3>&-

cd /data/ && mkjso --source_list_file=${SRC_FILES} --aot_out=${JSO_FILE}
rm -f ${SRC_FILES}
cd -
fi

# Rename caf2.cal to disable preload in caf2
if [ -f /usr/framework/yunos/caf2.cal ]; then
mv /usr/framework/yunos/caf2.cal /usr/framework/yunos/caf2.preload
fi
