#pragma once 

#if defined(_MSC_VER)
#include "new.h"
#endif

#include    <vector>
#include    <string>

#include    "DlisCommon.h"
#include    "DlisAllocator.h"
#include    "MemoryBuffer.h"
#include    "DLISFrame.h"


typedef void (*DlisNotifyCallback)(CDLISFrame *frame, void *params);

class CDLISParser
{
private:
    enum constants
    {
        Kb         = 1024,
        Mb         = Kb * Kb,
        FILE_CHUNK = 16 * Mb,

        MAX_ATTRIBUTE_LABEL       = 64,
        MAX_TEMPLATE_ATTRIBUTES   = 32,
        //
        STATE_PARSER_FIRST        = 0x00,
        STATE_PARSER_SET          = 0x01,
        STATE_PARSER_OBJECT       = 0x02,
        STATE_PARSER_TEMPLATE_ATTRIBUTE = 0x04,
        STATE_PARSER_ATTRIBUTE    = 0x08, 
        STATE_PARSER_ALL          = STATE_PARSER_SET | STATE_PARSER_OBJECT | STATE_PARSER_TEMPLATE_ATTRIBUTE | STATE_PARSER_ATTRIBUTE,
        //
        REP_CODE_VARIABLE_SIMPLE  = -1,
        REP_CODE_VARIABLE_COMPLEX = -2,
    };
    
    struct TemplateAttributes
    {
        RepresentationCodes  code;
        UINT                 count;
    };

    struct RepresentaionCodesLenght
    {
        RepresentationCodes  code;
        int                 length;
    };

    typedef unsigned char byte;
    //  ����� ����� 
    HANDLE              m_file;
    // ��������� DLIS
    StorageUnitLabel    m_storage_unit_label;

    // ���������� ������ ������ �� ����� DLIS    
    struct FileChunk : MemoryBuffer
    {
        size_t      pos;
        size_t      remaind;
        size_t      size_chunk;
        UINT64      file_remaind;
    };
    // ����� ������    
    FileChunk        m_file_chunk;

    // ��������� DLIS visible record
    struct VisibleRecord
    {
        char      *current;
        char      *end; 
        size_t     len;
    };

    // ��������� DLIS �������� 
    struct SegmentRecord
    {
        char            *current;
        char            *end;
        size_t           len;
    };

    struct FrameData
    {
        DlisValueObjName  obj_key;
        DlisChannelInfo  *channels;
        int               channel_count; 
        int               len;
        // 
        FrameData        *next;
    };

    // dlis ������, � ������� ��������
    VisibleRecord      m_visible_record;
    // ��������� ��������, ����������
    SegmentRecord      m_segment;
    SegmentHeader      m_segment_header;
    ComponentHeader    m_component_header;

    // ����� ��������� 
    UINT               m_state;
    // �������� ���� (root)
    DlisSet           *m_sets;
    // �������� ������ ��������� (next) ����� � ������
    // ���������� ��� ������� ������� � ������ DLIS
    DlisSet           **m_set_tail;
    DlisObject        **m_object_tail;
    DlisAttribute     **m_attribute_tail;
    DlisAttribute     **m_column_tail;
    DlisFrameData     **m_frame_tail;

    // ���������� (��������� ����������) �������  
    // ����� ��� �������� ���������� ������� �������� ��� ���������� ������ DLIS
    DlisSet           *m_last_set;
    DlisSet	          *m_last_root_set;
    DlisObject        *m_last_object;
    DlisAttribute     *m_last_attribute;
    DlisAttribute     *m_last_column;
    DlisFrameData     *m_last_frame;

    FrameData         *m_frame_data;

    CDLISAllocator     m_allocator;
    size_t             m_pull_id_strings;
    size_t             m_pull_id_objects;
    size_t             m_pull_id_frame_data;
    
    CDLISFrame         m_frame;

    DlisNotifyCallback  m_notify_frame_func;
    void               *m_notify_params;

private:
   static RepresentaionCodesLenght s_rep_codes_length[RC_LAST];
 
public:
    CDLISParser();
    ~CDLISParser();

    // ������� DLIS
    bool            Parse(const wchar_t *file_name);
    // �������������, �������� ���������� ������� � ������ �� �������
    bool            Initialize();
    void            Shutdown();

    DlisSet        *GetRoot()     { return m_sets; }

    void            CallbackNotifyFrame(DlisNotifyCallback func, void *params);

    char           *AttrGetString(DlisAttribute *attr, char *buf, size_t buf_len);
    int             AttrGetInt(DlisAttribute *attr);


    DlisAttribute  *FindColumnTemplate(DlisObject *object, DlisAttribute *attr);
    DlisAttribute  *FindAttribute(const DlisObject *object, const char *name_column);
    DlisSet        *FindSubSet(const char *name_sub_set, DlisSet *root = NULL);
    DlisObject     *FindObject(DlisValueObjName *obj, DlisSet *set);
    bool            ObjectNameCompare(const DlisValueObjName *left, const DlisValueObjName *rigth);


private:
    //  ������ �����
    bool            FileOpen(const wchar_t *file_name);
    bool            FileClose();
    bool            FileRead(char *data, DWORD len);
    UINT64          FileSize();
    // �������������� bit 2 littel endian
    inline void     Big2LittelEndian(void *dst, size_t len);
    void            Big2LittelEndianByte(byte *byte);

    // ������ ��������� � ���������� ������
    bool            ReadStorageUnitLabel();
    bool            ReadLogicalFiles();

    // ������ ����������� ������
    bool            BufferNext(char **data, size_t len);
    bool            BufferInitialize();
    bool            BufferIsEOF();
    bool            VisibleRecordNext();

    // ������ �������� DLIS ������ 
    bool            SegmentGet();
    bool            SegmentProcess(); 

    bool            SegmentLast(const SegmentHeader *segment_header);
    bool            SegmentFirst(const SegmentHeader *segment_header);

    // ������ ���������� DLIS
    bool            ComponentRead();
    bool            ComponentHeaderGet();

    // ������ ����� ������ DLIS
    bool            ReadRawData(void *dst, size_t len);
    bool            ReadCodeSimple(RepresentationCodes code, void **dst, size_t *len);
    bool            ReadCodeComplex(RepresentationCodes code, void *dst);

    bool            ReadIndirectlyFormattedLogicalRecord();
    // ������ ��������� ���������� DLIS
    bool            ReadSet();
    bool            ReadObject();
    bool            ReadAttribute();

    bool            ReadAttributeValue(DlisValue *attr_val, RepresentationCodes code, int type);

    void            SetAdd(DlisSet *set);
    void            ObjectAdd(DlisObject *obj);
    void            ColumnAdd(DlisAttribute *obj);
    void            AttributeAdd(DlisAttribute *obj);

    void            FlagsParserSet(UINT flag);
    char           *StringTrim(char *str, size_t *len);

    FrameData      *FrameDataBuild(DlisValueObjName *obj_name);
    bool            FrameDataParse(FrameData *frame);
    FrameData      *FrameDataFind(DlisValueObjName *obj_name);
};
