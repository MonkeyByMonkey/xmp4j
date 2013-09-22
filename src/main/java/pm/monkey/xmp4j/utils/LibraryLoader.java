package pm.monkey.xmp4j.utils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileFilter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class LibraryLoader {

    private static final LibraryLoader loader = new LibraryLoader();

    private final Collection<String> libraries;

    private LibraryLoader() {
        libraries = getLibraries();
    }

    private static Collection<String> getLibraries() {
        String[] classpath = System.getProperty("java.class.path", "").split(File.pathSeparator);

        List<String> libraries = new ArrayList<String>();

        for (String path : classpath) {
            try {
                File file = new File(path);

                if (file.isFile() && path.endsWith(".jar")) {
                    appendLibrariesFromJar(file, libraries);
                } else if (file.isDirectory()) {
                    appendLibrariesFromDirectory(file, libraries);
                }

            } catch (IOException e) { }
        }

        return libraries;
    }

    private static void appendLibrariesFromJar(File jarFile, Collection<String> libraries) throws IOException {

        JarFile jar = new JarFile(jarFile);
        Enumeration<JarEntry> entries = jar.entries();

        while (entries.hasMoreElements()) {
            JarEntry entry = entries.nextElement();

            if (isLibrary(entry)) {
                libraries.add(entry.getName());
            }
        }

    }

    private static void appendLibrariesFromDirectory(File dir, Collection<String> libraries) {

        File[] files = dir.listFiles(new FileFilter() {

            public boolean accept(File file) {
                return isLibrary(file);
            }
        });

        for (File file : files) {
            libraries.add(file.getPath());
        }
    }

    private static boolean isLibrary(File file) {
        return !file.isDirectory() && fileHasLibrarySuffix(file.getName());
    }

    private static boolean isLibrary(JarEntry entry) {
        return !entry.isDirectory() && fileHasLibrarySuffix(entry.getName());
    }

    private Collection<URL> resolveLibrary(String libName) {

        List<URL> urls = new ArrayList<URL>();
        String platformLibName = getPlatformLibraryName(libName);
        String qualifiedPlatformLibName = getPlatformLibraryName(libName + "." + getRuntimeArchitecture());

        for (String library : libraries) {

            File file = new File(library);

            if (platformLibName != null && file.getName().equals(platformLibName) ||
                qualifiedPlatformLibName != null && file.getName().equals(qualifiedPlatformLibName)) {

                URL libUrl = this.getClass().getClassLoader().getResource(library);

                if (libUrl != null) {
                    urls.add(libUrl);
                }
            }
        }

        return urls;
    }

    public static void loadLibrary(String libName) {

        try {
            System.loadLibrary(libName);
        } catch (UnsatisfiedLinkError e) {

            try {
                System.loadLibrary(libName + "." + getRuntimeArchitecture());
            } catch (UnsatisfiedLinkError ex) {
                loader.load(libName);
            }
        }

    }

    private void loadUrl(URL url) throws IOException {
        File file = null;
        InputStream in = null;
        OutputStream out = null;

        try {
            file = File.createTempFile("lib", ".lib");
            file.deleteOnExit();

            in = url.openStream();
            out = new BufferedOutputStream(new FileOutputStream(file));

            int len = 0;
            byte[] buffer = new byte[51200];
            while ((len = in.read(buffer)) > -1) {
                out.write(buffer, 0, len);
            }
            out.close();
            in.close();

            System.load(file.getAbsolutePath());

        } finally {

            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) { }

            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) { }

            if (file != null) {
                file.delete();
            }
        }
    }

    private void load(String libName) {

        Collection<URL> urls = resolveLibrary(libName);

        for (URL url : urls) {

            try {
                loadUrl(url);

                return;

            } catch (IOException e) { }
              catch (UnsatisfiedLinkError e) { }

        }

        throw new UnsatisfiedLinkError("no loadable " + libName + " anywhere");
    }

    private static boolean fileHasLibrarySuffix(String fileName) {
        return fileName.endsWith(".so") || fileName.endsWith(".dll") || fileName.endsWith(".dylib");
    }

    private static String getRuntimeArchitecture() {
        return System.getProperty("os.arch");
    }

    private static String getPlatformLibraryName(String libName) {
        String osName = System.getProperty("os.name", "unknown").toLowerCase();

        if (osName.startsWith("windows")) {
            return libName + ".dll";
        } else if (osName.startsWith("linux")) {
            return "lib" + libName + ".so";
        } else if (osName.startsWith("mac")) {
            return "lib" + libName + ".dylib";
        } else {
            return null;
        }
    }
}
