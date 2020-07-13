import "src/gui" as gui;

lib dlmain {
  srcs = ["dlmain.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper, reflection, base_widgets];
}

lib base_widgets {
  srcs = ["base-widgets.cc"];
  hdrs = ["base-widgets.h"];
  deps = [gui.widget_helper, gtk, reflection];
}

lib reflection {
  srcs = ["reflect.cc"];
  hdrs = ["reflect.h"];
}

solink "simdb.so" { deps = [dlmain]; }
