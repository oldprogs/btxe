// enumerations
enum file_type {flo_file, packet};
enum after_sending {keep, kill, truncate};
// Base Class of CPacket and CFloFiles . Attributes for both Packets and flo_files.
class CBaseFile {
  private:
    char priority;
    int type;
};
// CPacket is either a PKT or one of the entrys in a flofile and points to the next CPacket
class CPacket : public CBaseFile {
  private:
    String filename;
    unsigned long fileSize;
    int do_after;
    CPacket *next;
};
// CFloFile's attribute is a pointer to the first Packet.
class CFloFile : public CBaseFile {
  private:
    CPacket *firstFile;
};
// This class holds all (flo-)files and a pointer to the next address
class CAddress {
  private:
    FTNAddress adr;
    CBaseFile *files;
    CAddress *next;
};
// Start of the Chain of Addresses
class CNewOutbound {
  public:
    CNewOutbound
  private:
    CAddress *firstAddress;
};
