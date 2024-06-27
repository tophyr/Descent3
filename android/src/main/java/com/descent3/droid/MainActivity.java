package com.descent3.droid;

import org.libsdl.app.SDLActivity;

import java.sql.Array;
import java.util.ArrayList;
import java.util.Arrays;

public class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "Descent3",
        };
    }

    @Override
    protected String[] getArguments() {
      ArrayList<String> args = new ArrayList<>(Arrays.asList(super.getArguments()));
      args.add("-setdir");
      args.add("/sdcard/descent3");
      return args.toArray(new String[0]);
    }
}
