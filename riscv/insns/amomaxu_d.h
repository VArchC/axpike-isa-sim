require_extension('A');
require_rv64;
WRITE_RD(MMU.amo<uint64_t>(RS1, [&](uint64_t lhs) { return std::max(lhs, static_cast<reg_t>(RS2)); }));
