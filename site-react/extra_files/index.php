<?php
// Links to /index.php go back to the old site. A JavaScript redirect used to live
// here, which search engines read as a second copy of the home page rather than as
// a redirect; a 301 says plainly that the home page is the one address.
header('Location: https://xmq.sptk.net/', true, 301);
exit;
