<?php
//Load FUSE, if it isn't already loaded by php.ini
if (!extension_loaded("fuse"))
    dl("fuse.so");
error_reporting(E_ALL);

class Sample1Fuse extends Fuse {
}

$fuse = new Sample1Fuse();
$fuse->fuse_main($argc,$argv);
