<?php
//See https://github.com/jrk/passfs/blob/master/opts.c for an idea how the argopt stuff works

//Load FUSE, if it isn't already loaded by php.ini
if (!extension_loaded("fuse"))
    dl("fuse.so");
error_reporting(E_ALL);

class PHPRAMFS extends Fuse {
    //basic needed stuff
    public $name = "phpramfs";
    public $version = "1.3.37";
    
    //our own data
    public $show_dummyfile = false; //show '/dummyfile'
    public $dummyfile_n_name = ""; //if not empty, show '/dummyfile.VALUE'
    
    public function __construct() {
        $this->opt_keys = array_flip(array(
            "KEY_HELP",
            "KEY_FUSE_HELP",
            "KEY_VERSION",
            "KEY_DEBUG",
            "KEY_DUMMYFILE",
            "KEY_DUMMYFILE_I",
            "KEY_DUMMYFILE_S",
            "KEY_DUMMYFILE_N"
        ));
        $this->opts     = array(
            "--help" => $this->opt_keys["KEY_HELP"],
            "--version" => $this->opt_keys["KEY_VERSION"],
            "-h" => $this->opt_keys["KEY_HELP"],
            "-H" => $this->opt_keys["KEY_FUSE_HELP"],
            "-V" => $this->opt_keys["KEY_VERSION"],
            "dummyfile" => $this->opt_keys["KEY_DUMMYFILE"],
            "-d" => $this->opt_keys["KEY_DEBUG"],
            "-n " => $this->opt_keys["KEY_DUMMYFILE_N"]
        );
        $this->userdata = array(
            array(
                "templ" => "-st=%s",
                "value" => "",
                "key" => $this->opt_keys["KEY_DUMMYFILE_S"]
            ),
            array(
                "templ" => "-i=%lu",
                "value" => 0,
                "key" => $this->opt_keys["KEY_DUMMYFILE_I"]
            )
        );
    }
    public function main($argc, $argv) {
        $res = $this->opt_parse($argc, $argv, $this->userdata, $this->opts, array(
            $this,
            "opt_proc"
        ));
        if ($res === false) {
            printf("Error in opt_parse\n");
            exit;
        }
        $this->fuse_main($argc, $argv);
    }
    public function opt_proc(&$data, $arg, $key, &$argc, &$argv) {
        // return -1 to indicate error, 0 to accept parameter,1 to retain parameter and pase to FUSE
        switch ($key) {
            case FUSE_OPT_KEY_NONOPT:
                return 1;
                break;
            case $this->opt_keys["KEY_DUMMYFILE"]:
                printf("Showing dummy file\n");
                $this->show_dummyfile = true;
                
                return 0;
                break;
            case $this->opt_keys["KEY_FUSE_HELP"]:
                //Add a parameter to tell fuse to show its extended help
                array_push($argv, "-ho");
                $argc++;
            //No break, because we display our own help, and fuse adds its help then
            case $this->opt_keys["KEY_HELP"]:
                fprintf(STDERR, "%1\$s
Marco Schuster <marco@m-s-d.eu>

PHP-FUSE demo

Usage: %2\$s [options] mountpoint

Options:
    -o opt,[opt...]           mount options
    -h --help                 this help
    -H                        more help
    -V --version              print version info
    -d                        debug mode

Options specific to %1\$s:
    -o dummyfile              show an empty 'dummyfile' file in mountpoint root
    -s=content                set the content of the 'dummyfile.s'
    -i=size                   set the size of 'dummyfile.i' reported by getattr() (will be filled with nullbytes)
    -n name                   show a dummy file 'dummyfile.NAME' in mountpoint root

", $this->name, $argv[0]);
                
                return 0;
                break;
            case $this->opt_keys["KEY_VERSION"]:
                printf("%s %s\n", $this->name, $this->version);
                
                return 1;
                break;
            case $this->opt_keys["KEY_DEBUG"]:
                printf("debug mode enabled\n");
                
                return 1;
                break;
            case $this->opt_keys["KEY_DUMMYFILE_N"]:
                $arg = substr($arg, 3); //the -x value form strips the space between -x and the value
                printf("Showing dummy file 'dummyfile.%s'\n", $arg);
                $this->dummyfile_n_name = $arg;
                
                return 0;
                break;
            default:
                return 1;
        }
    }
    public function getdir($path, &$retval) {
        if ($path != "/") {
            return -FUSE_ENOENT;
        }
        $retval["."]        = array(
            'type' => FUSE_DT_DIR
        );
        $retval[".."]       = array(
            'type' => FUSE_DT_DIR
        );
        $retval["test.txt"] = array(
            'type' => FUSE_DT_REG
        );
        if ($this->dummyfile_n_name != "")
            $retval["dummyfile." . $this->dummyfile_n_name] = array(
                'type' => FUSE_DT_REG
            );
        if ($this->show_dummyfile)
            $retval["dummyfile"] = array(
                'type' => FUSE_DT_REG
            );
        if ($this->userdata[0]["value"] !== "")
            $retval["dummyfile.s"] = array(
                'type' => FUSE_DT_REG
            );
        if ($this->userdata[1]["value"] !== 0)
            $retval["dummyfile.i"] = array(
                'type' => FUSE_DT_REG
            );
        
        return 0;
    }
    
    public function getattr($path, &$st) {
        $st['dev']     = 0;
        $st['ino']     = 0;
        $st['mode']    = 0;
        $st['nlink']   = 0;
        $st['uid']     = 0;
        $st['gid']     = 0;
        $st['rdev']    = 0;
        $st['size']    = 0;
        $st['atime']   = 0;
        $st['mtime']   = 0;
        $st['ctime']   = 0;
        $st['blksize'] = 0;
        $st['blocks']  = 0;
        
        if ($path == "/") {
            $st['mode']  = FUSE_S_IFDIR | 0775;
            $st['nlink'] = 3;
            $st['size']  = 0;
        } elseif ($path == "/test.txt") {
            $st['mode']  = FUSE_S_IFREG | 0664;
            $st['nlink'] = 1;
            $st['size']  = 12;
        } elseif ($path == "/dummyfile") {
            $st['mode']  = FUSE_S_IFREG | 0664;
            $st['nlink'] = 1;
            $st['size']  = 0;
        } elseif ($path == "/dummyfile.i") {
            $st['mode']  = FUSE_S_IFREG | 0664;
            $st['nlink'] = 1;
            $st['size']  = (int) $this->userdata[1]["value"];
        } elseif ($path == "/dummyfile.s") {
            $st['mode']  = FUSE_S_IFREG | 0664;
            $st['nlink'] = 1;
            $st['size']  = strlen($this->userdata[0]["value"]);
        } elseif ($path == "/dummyfile." . $this->dummyfile_n_name) {
            $st['mode']  = FUSE_S_IFREG | 0664;
            $st['nlink'] = 1;
            $st['size']  = 0;
        }
        
        return 0;
    }
    public function open($path, $mode) {
        return 1;
    }
    public function read($path, $fh, $offset, $buf_len, &$buf) {
        if ($path == "/test.txt") {
            $buf = "hello world\n";
        } elseif ($path == "/dummyfile") {
            $buf = "";
        } elseif ($path == "/dummyfile.i") {
            $buf = str_pad("", $this->userdata[1]["value"]);
        } elseif ($path == "/dummyfile.s") {
            $buf = $this->userdata[0]["value"];
        } elseif ($path == "/dummyfile." . $this->dummyfile_n_name) {
            $buf = "";
        }
        
        return strlen($buf);
    }
    
    public function release($path, $fh) {
        return 0;
    }
}

$fuse = new PHPRAMFS();
$fuse->main($argc, $argv);
