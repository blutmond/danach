import "src/gui" as gui;

lib dlmain {
  srcs = ["dlmain.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper];
}

solink "notes.so" { deps = [dlmain]; }

lib serialize {
  srcs = ["serialize.cc"];
  hdrs = ["serialize.h"];
}

lib generated {
  srcs = ["generated.cc", "callable.cc"];
  hdrs = ["callable.h", "generated.h", "type-info.h", "type-support.h"];
  deps = [
    gtk, gui.so_handoff, gui.widget_helper,
          gui.buffer_view, serialize];
}

lib dlmain_toy {
  srcs = ["dlmain2.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper, generated,
          gui.buffer_view, serialize];
}

solink "notes_toy.so" { deps = [dlmain_toy]; }
