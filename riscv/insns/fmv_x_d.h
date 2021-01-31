require_extension('D');
require_rv64;
require_fp;
WRITE_RD(static_cast<freg_t>(FRS1).v[0]);
