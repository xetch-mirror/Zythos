.hfs_nodes : {
    __start_hfs_nodes = .;
    KEEP(*(hfs_nodes))
    __stop_hfs_nodes = .;
}