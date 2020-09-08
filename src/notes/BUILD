import "src/gui" as gui;

lib dlmain {
  srcs = ["dlmain.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper];
}

solink "notes.so" { deps = [dlmain]; }

lib generated {
  srcs = ["generated.cc", "callable.cc", "serialize.cc", "type-support.cc",
          "unary-draw-functor.cc"];
  hdrs = ["callable.h", "generated.h", "type-info.h", "type-support.h",
          "serialize.h"];
  deps = [
    gtk, gui.so_handoff, gui.widget_helper,
          gui.buffer_view];
}

lib dlmain_toy {
  srcs = ["dlmain2.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper, generated,
          gui.buffer_view];
}

solink "notes_toy.so" { deps = [dlmain_toy]; }
