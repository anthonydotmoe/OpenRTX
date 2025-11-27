project('tinyusb', 'c')

tinyusb_inc = include_directories('openrtx', 'src', 'hw')

tinyusb_src = ['src/tusb.c',
               'src/common/tusb_fifo.c',
               'src/device/usbd.c',
               'src/device/usbd_control.c',
               'src/class/cdc/cdc_device.c',
               'src/class/msc/msc_device.c',
               'src/portable/synopsys/dwc2/dcd_dwc2.c',
               'src/portable/synopsys/dwc2/dwc2_common.c']

tinyusb_dep = declare_dependency(
    include_directories : tinyusb_inc,
    sources             : tinyusb_src,
    compile_args        : ['-w']
)
