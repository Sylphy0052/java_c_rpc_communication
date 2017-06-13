#include <stdio.h>

#include "java_rules_of_the_grammer.h"

// void hexdump(char *desc, size_t len) {
//     int i;
//     unsigned char buff[17];
//     int flag = 0;
//     size_t pointer;
//     if (pc < len) {
//         pointer = 0;
//         flag = 1;
//     } else {
//         pointer = pc - len;
//     }
//     if(desc != NULL)
//         printf("%s:\n", desc);
//         if(len == 0) {
//     printf("  ZERO LENGTH\n");
//     }
//
//     printf("pc : %d\n", pointer);
//     for (i = pointer; i < pointer + len; i++) {
//         if(flag == 1) {
//             if ((i % 16) == 0) {
//                 if (i != 0) {
//                     printf("  %s\n", buff);
//                 }
//                 printf("  %04x ", i);
//             }
//         }
//         printf(" %02x", b[i]);
//         if((b[i] < 0x20) || (b[i] > 0x7e)) {
//             buff[i % 16] = '.';
//         } else {
//             buff[i % 16] = b[i];
//         }
//         buff[(i % 16) + 1] = '\0';
//     }
//     while((i % 16) != 0) {
//         printf("   ");
//         i++;
//     }
//     while((i % 16) != 0) {
//         printf("   ");
//         i++;
//     }
//     printf ("  %s\n", buff);
// }

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



// unsigned char read_n_byte(unsigned char *bytes, size_t len) {
//     unsigned char *result = malloc(sizeof(unsigned char) * len);
//     int i;
//     for(i = pc; i < len; i++) {
//         result[i - pc] = b[i];
//     }
//     printf("result[0] : %02x, b[pc] : %02x\n", result, bytes[0]);
//     pc += len;
//     return result;
// }
//
// void analyze_classname() {
//     // utf
//     size_t header_len = 0;
//     size_t len = 2;
//     int i;
//     for(i = 0; i < len; i++) {
//         header_len <<= 8;
//         header_len += b[pc++];
//     }
//     printf("className : ");
//     char *ret = malloc(sizeof(unsigned char) * (header_len + 1));
//     for(i = 0; i < header_len; i++) {
//         memset(&ret[i], b[pc++], 1);
//         printf("%s", &ret[i]);
//     }
//     memset(&ret[i], "\0", 1);
//     printf("\n");
// }
//
// void analyze_serialversionuid() {
//     // long
//     int i;
//     size_t len = 8;
//     unsigned char *uid = malloc(sizeof(unsigned char) * (len + 1));
//     printf("SerialVersionUID : ");
//     for(i = 0; i < len; i++) {
//         memset(&uid[i], b[pc++], 1);
//         printf("%02x", uid[i]);
//     }
//     printf("\n");
//     // printf("SerialVersionUID : %016x\n", &uid);
// }
//
// void analyze_classdescinfo() {
//     // classDescFlags fields classAnnotation superClassDesc
//
// }
//
// void analyze_newclassdesc() {
//     switch(b[pc++]) {
//     case TC_CLASSDESC:
//         analyze_classname();
//         analyze_serialversionuid();
//         analyze_newhandle();
//         analyze_classdescinfo();
//     case TC_PROXYCLASSDESC:
//         printf("undefined\n");
//     }
// }
//
// void analyze_classdesc() {
//     printf("analyze_classdesc\n");
//     switch(b[pc]) {
//     // newClassDesc | nullReference | (ClassDesc)prevObject
//     case TC_CLASSDESC:
//         analyze_newclassdesc();
//         break;
//     default:
//         printf("undefined\n");
//     }
// }
//
// void analyze_newhandle() {
//     // The next number in sequence is assigned
//     // to the object being serialized or deserialized
//     printf("analyze_newhandle\n");
//
// }
//
// void analyze_value() {
//     printf("analyze_value\n");
// }
//
// void analyze_newobject() {
//     printf("analyze_newobject\n");
//     analyze_classdesc();
//     analyze_newhandle();
//     analyze_value();
// }
//
// void analyze_blockdata() {
//     printf("analyze_blockdata\n");
// }
//
// void analyze_object() {
//     printf("analyze_object\n");
//     switch(b[pc++]) {
//     case TC_OBJECT:
//         analyze_newobject();
//         break;
//     default:
//         printf("undefined\n");
//     }
// }

char *read_nibble(unsigned char *bytes) {
    unsigned char *ret = malloc(sizeof(unsigned char));
    ret = bytes[0] >> 4;
    return ret;
}

char *read_n_byte(unsigned char *bytes, size_t len) {
    int i;
    char *ret = malloc(sizeof(char) * (len + 1));
    for(i = 0; i < len; i++) {
        memset(&ret[i], bytes[i], 1);
    }
    memset(&ret[i], "\0", 1);
    return ret;
}

size_t analyze_classname(struct classname *cn, unsigned char *bytes) {
    size_t len = 0;
    cn = malloc(sizeof(struct classname));
    // utf
    size_t handler_len = 0;
    int i;
    for(i = 0; i < UTF_LENGTH; i++) {
        handler_len <<= BYTE_LENGTH;
        // printf("%02x\n", bytes[len]);
        handler_len += bytes[len++];
    }
    cn->name = read_n_byte(&bytes[len], handler_len);
    hexdump("classname", cn->name, handler_len);
    len += handler_len;
    return len;
}

size_t analyze_serialversionuid(struct serialversionuid *uid, unsigned char *bytes) {
    size_t len = 0;
    // long
    unsigned long id;
    for(len = 0; len < UID_LENGTH; len++) {
        id <<= BYTE_LENGTH;
        // printf("bytes[%d] : %02x\n", len, bytes[len]);
        id += bytes[len];
    }
    uid->uid = id;
    printf("SerialVersionUID : 0x%16lx\n", uid->uid);
    return len;
}

size_t analyze_classdescflags(struct classdescflags *cdf, unsigned char *bytes) {
    size_t len = 0;
    cdf = malloc(sizeof(struct classdescflags));
    cdf->b = bytes[len++];
    hexdump("classdescflags", &cdf->b, 1);
    return len;
}

size_t analyze_primtypecode(struct primtypecode *ptc, unsigned char *bytes) {
    size_t len = 0;
    ptc = malloc(sizeof(struct primtypecode));
    ptc = bytes[len++];
    printf("prim_type code : %s\n", &ptc);
    return len;
}

size_t analyze_fieldname(struct fieldname *fn, unsigned char *bytes) {
    size_t len = 0;
    fn = malloc(sizeof(struct fieldname));
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

size_t analyze_premitivedesc(struct primitivedesc *pd, unsigned char *bytes) {
    size_t len = 0;
    pd = malloc(sizeof(struct primitivedesc));
    len += analyze_primtypecode(&pd->ptc, &bytes[len]);
    len += analyze_fieldname(&pd->fn, &bytes[len]);
    return len;
}

size_t analyze_fielddesc(struct fielddesc *fd, unsigned char *bytes, struct fielddesc *prevfd) {
    size_t len = 0;
    switch(bytes[len]) {
    case 'I': //Integer
        len += analyze_premitivedesc(&fd->u.pd, &bytes[len]);
        break;
    default:
        hexdump("Undefined -fielddesc-", &bytes[len], 1);
    }
    fd->next = NULL;
    if(prevfd != NULL) {
        prevfd->next = fd;
    }
    return len;
}

// void fielddesc_next() {
//
// }

size_t analyze_fields(struct fields *f, unsigned char *bytes) {
    int len = 0;
    f = malloc(sizeof(struct fields));
    unsigned short count;
    count = (bytes[len++] << 8) + bytes[len++];
    f->count = count;
    printf("count : %d\n", f->count);
    struct fielddesc *prevfd = malloc(sizeof(struct fielddesc));
    struct fielddesc *fd = malloc(sizeof(struct fielddesc));
    prevfd = NULL;
    for(int i = 0; i < count; i++) {
        printf("i : %d / len : %d\n", i, len);
        len += analyze_fielddesc(&fd, &bytes[len], &prevfd);
        prevfd = fd;
    }
    printf("fields fin\n");
    return len;
}

size_t analyze_classannotation(struct classannotation *ca, unsigned char *bytes) {
    size_t len = 0;
    ca = malloc(sizeof(struct classannotation));

    return len;
}

size_t analyze_superclassdesc(struct superclassdesc *scd, unsigned char *bytes) {
    size_t len = 0;
    scd = malloc(sizeof(struct superclassdesc));

    return len;
}

size_t analyze_classdescinfo(struct classdescinfo *cdi, unsigned char *bytes) {
    size_t len = 0;
    cdi = malloc(sizeof(struct classdescinfo));
    len += analyze_classdescflags(&cdi->cdf, bytes);
    len += analyze_fields(&cdi->f, &bytes[len]);
    len += analyze_classannotation(&cdi->ca, &bytes[len]);
    len += analyze_superclassdesc(&cdi->scd, &bytes[len]);
    return len;
}

size_t analyze_newclassdesc(struct newclassdesc *ncd, unsigned char *bytes) {
    size_t len = 0;
    ncd = malloc(sizeof(struct newclassdesc));
    switch(bytes[0]) {
    case TC_CLASSDESC:
        len++;
        len += analyze_classname(&ncd->cn, &bytes[len]);
        len += analyze_serialversionuid(&ncd->uid, &bytes[len]);
        len += analyze_newhandle(&ncd->nh, &bytes[len]);
        len += analyze_classdescinfo(&ncd->cdi, &bytes[len]);
        break;

    default:
        hexdump("Undefined -newclassdesc-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_classdesc(struct classdesc *cd, unsigned char *bytes) {
    size_t len = 0;
    cd = malloc(sizeof(struct classdesc));
    switch(bytes[0]) {
    case TC_CLASSDESC:
        len += analyze_newclassdesc(&cd->u.ncd, bytes);
        break;
    default:
        hexdump("Undefined -classdesc-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_newhandle(struct newhandle *nh, unsigned char *bytes) {
    size_t len = 0;
    nh = malloc(sizeof(struct newhandle));
    nh->handle = read_nibble(bytes);
    return len;
}

size_t analyze_newobject(struct newobject *no, unsigned char *bytes) {
    size_t len = 0;
    no = malloc(sizeof(struct newobject));
    int i = 0;
    len += analyze_classdesc(&no->cd, bytes);
    len += analyze_newhandle(&no->nh, &bytes[len]);
    // while(1) {
    //     len += analyze_classdata(no->cd[i++], &bytes[len]);
    // }
}

size_t analyze_object(struct object *o, unsigned char *bytes) {
    size_t len = 0;
    o = malloc(sizeof(struct object));
    switch(bytes[0]) {
    case TC_OBJECT:
        len = analyze_newobject(&o->u.no, &bytes[1]);
        len++;
        break;

    // case TC_RESET:

    default:
        hexdump("Undefined -object-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_content(struct content *c, unsigned char *bytes) {
    size_t len = 0;
    switch(bytes[0]) {
    case TC_OBJECT:
        c = malloc(sizeof(struct content));
        len = analyze_object(&c->u.o, bytes);
        break;

    // case TC_BLOCKDATA:
    // case TC_BLOCKDATALONG:
    //     len = analyze_blockdata(s->b, bytes);
    //     break;
    default:
        hexdump("Undefined -content-", bytes, 1);
        break;
    }
    return len;
}

size_t analyze_magic(struct magic *m, unsigned char *bytes) {
    size_t len = 2;
    m->stread_magic = malloc(sizeof(unsigned char) * len);
    memcpy(m->stread_magic, bytes, len);
    hexdump("magic", m->stread_magic, len);
    return len;
}

size_t analyze_version(struct version *v, unsigned char *bytes) {
    size_t len = 2;
    v->stream_version = malloc(sizeof(unsigned char) * len);
    memcpy(v->stream_version, bytes, len);
    hexdump("version", v->stream_version, len);
    return len;
}

size_t analyze_contents(struct contents *c, unsigned char *bytes, size_t len) {
    int i = 0;
    size_t ret = 0;
    switch(bytes[ret]) {
    case TC_OBJECT:
        c = malloc(sizeof(struct contents));
        ret += analyze_content(&c->c, &bytes[ret]);
        break;
    default:
        hexdump("Undefined -contents-", &bytes[ret], 1);
        break;
    }

    // while(ret < len) {
    //     switch(bytes[ret]) {
    //     case TC_OBJECT:
    //         c = malloc(sizeof(struct contents));
    //         ret += analyze_content(&c->c[i++], &bytes[ret]);
    //         break;
    //     default:
    //         printf("Undefined -contents- : %02x\n", &bytes[ret]);
    //         break;
    //     }
    // }
    return ret;
}

void analyze_stream(struct stream *s, unsigned char *bytes, size_t len) {
    size_t pc = 0;
    pc += analyze_magic(&s->m, bytes);
    pc += analyze_version(&s->v, &bytes[pc]);
    pc += analyze_contents(&s->c, &bytes[pc], len);
    // return s;
}

void analyze_grammer(unsigned char *bytes, size_t len) {
    struct stream *s = malloc(sizeof(struct stream));
    hexdump("hexdump", bytes, len);
    // struct stream s;
    analyze_stream(s, bytes, len);
}
