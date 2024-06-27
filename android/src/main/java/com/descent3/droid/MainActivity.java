package com.descent3.droid;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.IOException;
import java.sql.Array;
import java.util.ArrayList;
import java.util.Arrays;

public class MainActivity extends SDLActivity {
  @Override
  protected void onStart() {
    super.onStart();

    File test = new File("/data/local/tmp/descent3/custom/cache/foo");
    try {
      test.createNewFile();
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
    test.deleteOnExit();
  }

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
      args.add("/data/local/tmp/descent3");
      return args.toArray(new String[0]);
    }
}
