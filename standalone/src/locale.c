const char l_usage[] = (
    "Usage:\n\n"
    "%s [-hv?] [-o OUTFILE] [-p PROPERTY=VALUE [-p ...]] [INFILE]\n"
);
const char l_version[] = "mintpl-cli version %s\nlibmintpl version %s\n";

const char l_err_mtpl_init_failed[] = "libmtpl init failed, error code %d\n"; 
const char l_err_unknown_opt[] = "Unknown option: %c\n";
const char l_err_open_out_failed[] = "Failed to open '%s' for writing\n";
const char l_err_open_in_failed[] = "Failed to open '%s' for reading\n";
const char l_err_malformed_prop[] = "Malformed property: %s\n";
const char l_err_set_prop[] = "Setting property failed, error code %d\n";
const char l_err_read_bytes[] = "Failed to read %d bytes\n";
const char l_err_parse[] = (
    "Failed to parse template, error code %d (position %d)\n"
);
const char l_err_write[] = "Failed to write %d bytes\n";

