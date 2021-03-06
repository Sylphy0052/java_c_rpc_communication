#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "java_rules_of_the_grammer.h"
#include "handle.h"

void hexdump(const char *desc, const unsigned char *pc, const size_t len) {
  int i;
  unsigned char buff[17];
  if (desc != NULL)
    printf ("%s:\n", desc);
  if (len == 0) {
    printf("  ZERO LENGTH\n");
    return;
  }
  for (i = 0; i < len; i++) {
    if ((i % 16) == 0) {
      if (i != 0)
        printf ("  %s\n", buff);
      printf ("  %04x ", i);
    }
    printf (" %02x", pc[i]);
    if ((pc[i] < 0x20) || (pc[i] > 0x7e))
      buff[i % 16] = '.';
    else
      buff[i % 16] = pc[i];
    buff[(i % 16) + 1] = '\0';
  }
  while ((i % 16) != 0) {
    printf ("   ");
    i++;
  }
  printf ("  %s\n", buff);
}

unsigned char *read_n_byte(const unsigned char *bytes, size_t len) {
    int i;
    unsigned char *ret = malloc(sizeof(unsigned char) * (len + 1));
    memcpy(ret, bytes, len);
    memset(&ret[len], '\0', 1);
    return ret;
}

size_t analyze_classname(struct classname *cn, const unsigned char *bytes) {
    size_t len = 0;
    // utf
    size_t handler_len = 0;
    int i;
    for(i = 0; i < UTF_LENGTH; i++) {
        handler_len <<= BYTE_LENGTH;
        handler_len += bytes[len++];
    }
    cn->name = read_n_byte(&bytes[len], handler_len);
    hexdump("classname", cn->name, handler_len);
    len += handler_len;
    return len;
}

size_t analyze_serialversionuid(struct serialversionuid *uid, const unsigned char *bytes) {
    size_t len = 0;
    // long
    unsigned long id;
    for(len = 0; len < UID_LENGTH; len++) {
        id <<= BYTE_LENGTH;
        id += bytes[len];
    }
    uid->uid = id;
    printf("SerialVersionUID : 0x%16lx\n", uid->uid);
    return len;
}

size_t analyze_classdescflags(struct classdescflags *cdf, const unsigned char *bytes) {
    size_t len = 1;
    cdf->b = read_n_byte(&bytes[0], len);
    return len;
}

size_t analyze_primtypecode(struct primtypecode *ptc, const unsigned char *bytes) {
    size_t len = 1;
    ptc->code = read_n_byte(&bytes[0], len);
    printf("prim_type_code : %s\n", ptc->code);
    return len;
}

size_t analyze_fieldname(struct fieldname *fn, const unsigned char *bytes) {
    size_t len = 0;
    // utf
    size_t handler_len = 0;
    for(int i = 0; i < UTF_LENGTH; i++) {
        handler_len <<= BYTE_LENGTH;
        handler_len += bytes[len++];
    }
    fn->name = read_n_byte(&bytes[len], handler_len);
    hexdump("fieldname", fn->name, handler_len);
    len += handler_len;
    return len;
}

size_t analyze_premitivedesc(struct primitivedesc *pd, const unsigned char *bytes) {
    size_t len = 0;
    len += analyze_primtypecode(&pd->ptc, &bytes[len]);
    len += analyze_fieldname(&pd->fn, &bytes[len]);
    return len;
}

size_t analyze_objtypecode(struct objtypecode *otc, const unsigned char *bytes) {
    size_t len = 1;
    otc->code = read_n_byte(&bytes[0], len);
    printf("obj_type_code : %s\n", otc->code);
    return len;
}

size_t analyze_classname1(struct classname1 *cn1, const unsigned char *bytes) {
    size_t len = 0;
    len += analyze_object(&cn1->o, &bytes[len]);
    return len;
}

size_t analyze_objectdesc(struct objectdesc *od, const unsigned char *bytes) {
    size_t len = 0;
    len += analyze_objtypecode(&od->otc, &bytes[len]);
    len += analyze_fieldname(&od->fn, &bytes[len]);
    len += analyze_classname1(&od->cn1, &bytes[len]);
    return len;
}

size_t analyze_fielddesc(struct fielddesc **fd, const unsigned char *bytes) {

    struct fielddesc *fd_ = malloc(sizeof(struct fielddesc));
    size_t len = 0;
    fd_->next = NULL;
    switch(bytes[len]) {
    case 'I': //Integer
    case 'B': //Byte
        fd_->type = PRIMITIVE;
        len += analyze_premitivedesc(&fd_->u.pd, &bytes[len]);
        break;
    case 'L': //Object
        fd_->type = OBJECT;
        len += analyze_objectdesc(&fd_->u.od, &bytes[len]);
        break;
    default:
        hexdump("Undefined -fielddesc-", &bytes[len], 1);
    }

    if(*fd == NULL) {
        *fd = fd_;
    } else {
        struct fielddesc *f = *fd;

        while(f->next != NULL) {
            f = f->next;
        }
        f->next = fd_;
        f->next->next = NULL;
    }
    return len;
}

size_t analyze_fields(struct fields *f, const unsigned char *bytes) {
    size_t len = 0;
    unsigned short count;
    count = (bytes[len++] << 8);
    count += bytes[len++];
    f->count = count;
    f->fd = NULL;
    for(size_t i = 0; i < f->count; i++) {
        len += analyze_fielddesc(&f->fd, &bytes[len]);
    }
    return len;
}

size_t analyze_classannotation(struct classannotation *ca, const unsigned char *bytes) {
    size_t len = 0;
    switch(bytes[len]) {
    case TC_ENDBLOCKDATA:
        printf("END_BLOCKDATA\n");
        len++;
        break;
    default:
        // len += analyze_contents(&ca->c, &bytes[len]);
        // printf("END_BLOCKDATA\n");
        // len++;
        hexdump("Undefined -classannotation-", &bytes[len], 1);
        break;
    }
    return len;
}

size_t analyze_superclassdesc(struct superclassdesc *scd, const unsigned char *bytes) {
    size_t len = 0;
    hexdump("analyze_superclassdesc", &bytes[len], 10);
    len += analyze_classdesc(&scd->cd, &bytes[len]);
    return len;
}

void analyze_newhandle_ncd(struct newclassdesc *ncd) {
    ncd->nh.handle = new_handle_ncd(ncd);
    printf("newHandle_ncd : %x\n", ncd->nh.handle);
}

size_t analyze_classdescinfo(struct classdescinfo *cdi, const unsigned char *bytes) {
    size_t len = 0;
    len += analyze_classdescflags(&cdi->cdf, bytes);
    len += analyze_fields(&cdi->f, &bytes[len]);
    len += analyze_classannotation(&cdi->ca, &bytes[len]);
    len += analyze_superclassdesc(&cdi->scd, &bytes[len]);
    return len;
}

size_t analyze_newclassdesc(struct newclassdesc *ncd, const unsigned char *bytes) {
    size_t len = 0;
    switch(bytes[0]) {
    case TC_CLASSDESC:
        len++;
        len += analyze_classname(&ncd->cn, &bytes[len]);
        len += analyze_serialversionuid(&ncd->uid, &bytes[len]);
        analyze_newhandle_ncd(ncd);
        len += analyze_classdescinfo(&ncd->cdi, &bytes[len]);
        break;

    default:
        hexdump("Undefined -newclassdesc-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_classdesc(struct classdesc **cd, const unsigned char *bytes) {
    struct classdesc *cd_ = *cd = malloc(sizeof(struct classdesc));
    size_t len = 0;
    switch(bytes[0]) {
    case TC_CLASSDESC:
        len += analyze_newclassdesc(&cd_->u.ncd, bytes);
        break;
    case TC_NULL:
        printf("NullReference\n");
        len++;
        break;
    default:
        hexdump("Undefined -classdesc-", bytes, 1);
        break;
    }
    return len;
}

void analyze_newhandle_no(struct newobject *no) {
    no->nh.handle = new_handle_no(no);
    printf("newHandle_no : %x\n", no->nh.handle);
}

size_t analyze_classdata(struct classdata **cds, const unsigned char *bytes, struct fielddesc *fd) {
    size_t len = 0;
    unsigned char code;
    if(fd->type == PRIMITIVE) {
        code = fd->u.pd.ptc.code[0];
    } else {
        code = fd->u.od.otc.code[0];
    }
    struct classdata *cds_ = malloc(sizeof(struct classdata));
    cds_->next = NULL;
    int ret;
    switch(code) {
    case 'I':
        ret = 0;
        len = 4;
        for (size_t i = 0; i < len; i++) {
            ret <<= BYTE_LENGTH;
            ret += bytes[i];
        }
        cds_->u.i = ret;
        printf("Integer : %x\n", cds_->u.i);
        break;
    case 'B':
        cds_->u.b = bytes[len++];
        printf("Byte : %x\n", cds_->u.b);
        break;
    case 'L':
        len += analyze_object(&cds_->u.o, &bytes[len]);
        break;
    default:
        hexdump("Undefined -classdata-", bytes, 1);
    }

    if(*cds == NULL) {
        *cds = cds_;
    } else {
        struct classdata *c = *cds;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = cds_;
        c->next->next = NULL;
    }
    return len;
}

size_t analyze_newobject(struct newobject *no, const unsigned char *bytes) {
    size_t len = 0;
    len += analyze_classdesc(&no->cd, bytes);
    analyze_newhandle_no(no);
    struct fielddesc *fd = no->cd->u.ncd.cdi.f.fd;

    no->cds = NULL;
    while(fd != NULL) {
        len += analyze_classdata(&no->cds, &bytes[len], fd);
        fd = fd->next;
    }
    return len;
}

void analyze_newhandle_ns(struct newstring *ns) {
    ns->nh.handle = new_handle_ns(ns);
    printf("newHandle_ns : 0x%x\n", ns->nh.handle);
}

size_t analyze_newstring(struct newstring *ns, const unsigned char *bytes) {
    size_t len = 0;
    switch(bytes[len++]) {
    case TC_STRING:
        analyze_newhandle_ns(ns);
        // utf
        size_t handler_len = 0;
        for(int i = 0; i < UTF_LENGTH; i++) {
            handler_len <<= BYTE_LENGTH;
            handler_len += bytes[len++];
        }
        printf("handler_len : %zu\n", handler_len);
        ns->utf = read_n_byte(&bytes[len], handler_len);
        len += handler_len;
        break;
    case TC_LONGSTRING:
        break;
    default:
        hexdump("Undefined -newstring-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_prevobject(struct prevobject *po, const unsigned char *bytes) {
    size_t len = 0;
    // int
    unsigned int handle = 0;
    for(int i = 0; i < 4; i++) {
        handle <<= BYTE_LENGTH;
        handle += bytes[len++];
    }
    printf("handle : %x\n", handle);
    po->h = gethandle(handle);
    return len;
}


size_t analyze_object(struct object **o, const unsigned char *bytes) {
    struct object *o_ = *o = malloc(sizeof(struct object));
    size_t len = 0;
    switch(bytes[0]) {
    case TC_OBJECT:
        len = analyze_newobject(&o_->u.no, &bytes[1]);
        len++;
        break;
    case TC_STRING:
        len = analyze_newstring(&o_->u.ns, &bytes[len]);
        break;
    case TC_REFERENCE:
        len = analyze_prevobject(&o_->u.po, &bytes[1]);
        printf("TC_REFERENCE\n");
        len++;
        break;
    // case TC_RESET:

    default:
        hexdump("Undefined -object-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_content(struct content *c, const unsigned char *bytes) {
    size_t len = 0;
    switch(bytes[0]) {
    case TC_OBJECT:
        len = analyze_object(&c->u.o, bytes);
        break;

    case TC_REFERENCE:
        printf("TC_REFERENCE\n");
        break;

    // case TC_BLOCKDATA:
    // case TC_BLOCKDATALONG:
    //     len = analyze_blockdata(s->b, bytes);
    //     break;
    default:
        hexdump("Undefined -content-", bytes, 5);
        break;
    }
    return len;
}

size_t analyze_magic(struct magic *m, const unsigned char *bytes) {
    size_t len = 2;
    m->stread_magic = read_n_byte(&bytes[0], 2);
    hexdump("magic", m->stread_magic, len);
    return len;
}

size_t analyze_version(struct version *v, const unsigned char *bytes) {
    size_t len = 2;
    v->stream_version = read_n_byte(&bytes[0], 2);
    hexdump("version", v->stream_version, len);
    return len;
}

size_t analyze_contents(struct contents **c, const unsigned char *bytes, size_t len) {
    struct contents *c_ = *c = malloc(sizeof(struct contents));
    size_t ret = 0;
    while(ret < len) {
        printf("len : %zu ret : %zu\n", len, ret);
        switch(bytes[ret]) {
        case TC_OBJECT:
        case TC_REFERENCE:
            ret += analyze_content(&c_->c, &bytes[ret]);
            break;
        default:
            hexdump("Undefined -contents-", &bytes[ret], 1);
            exit(1);
            ret += 100;
            break;
        }
    }
    return ret;
}

void analyze_stream(struct stream **s, const unsigned char *bytes, size_t len) {
    struct stream *s_ = *s = malloc(sizeof(struct stream));
    size_t pc = 0;
    pc += analyze_magic(&s_->m, bytes);
    pc += analyze_version(&s_->v, &bytes[pc]);
    pc += analyze_contents(&s_->c, &bytes[pc], len - pc);
}

struct stream analyze_grammer(const unsigned char *bytes, size_t len) {
    struct stream *s;
    hexdump("hexdump", bytes, len);
    analyze_stream(&s, bytes, len);
    return *s;
}
