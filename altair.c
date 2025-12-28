#include "sim_defs.h"
#include "altair_defs.h"

/* обязательные глобалы SIMH */
int32 sim_interval = 0;
t_bool sim_is_running = FALSE;
t_bool sim_step = FALSE;
/* breakpoints */
int32 sim_brk_summ = 0;
uint32 sim_brk_types = 0;
uint32 sim_brk_dflt  = 0;

/* устройства Altair */
extern DEVICE *sim_devices[];

/* CPU loop */
extern t_stat sim_instr (void);

static void sim_reset_all(void)
{
    for (DEVICE **dptr = sim_devices; *dptr != NULL; dptr++) {
        if ((*dptr)->reset)
            (*dptr)->reset(*dptr);
    }
}

t_stat sim_process_event(void)
{
    return SCPE_OK;
}

/* timing / idle */
t_bool sim_idle (uint32 tmr, int sin_cyc) {
    (void)tmr;
    (void)sin_cyc;
    return FALSE;    
}
uint32 sim_os_msec(void) { return 0; }

t_stat sim_putchar (int32 c)
{
    putchar((unsigned char)c);
    return SCPE_OK;
}

int32 sim_brk_test (t_addr addr, uint32 mask)
{
    (void)addr;
    (void)mask;
    return 0;   /* breakpoint not hit */
}

t_bool get_yn (const char *ques, t_bool deflt)
{
    (void)ques;
    return deflt;
}

t_stat sim_activate (UNIT *uptr, int32 delay)
{
    (void)uptr;
    (void)delay;
    return SCPE_OK;
}

t_stat sim_cancel (UNIT *uptr)
{
    (void)uptr;
    return SCPE_OK;
}

const char *get_glyph (const char *iptr, char *optr, char mchar)
{
    char *op = optr;

    /* skip leading whitespace */
    while (*iptr && isspace((unsigned char)*iptr))
        iptr++;

    /* copy until whitespace or mchar */
    while (*iptr &&
           !isspace((unsigned char)*iptr) &&
           (mchar == 0 || *iptr != mchar))
    {
        *op++ = *iptr++;
    }

    *op = '\0';
    return iptr;
}

t_stat sim_decode_quoted_string (const char *iptr, uint8 *optr, uint32 *osize)
{
char quote_char;
uint8 *ostart = optr;

*osize = 0;
if ((strlen(iptr) == 1) ||
    (iptr[0] != iptr[strlen(iptr)-1]) ||
    ((iptr[strlen(iptr)-1] != '"') && (iptr[strlen(iptr)-1] != '\'')))
    return SCPE_ARG;            /* String must be quote delimited */
quote_char = *iptr++;           /* Save quote character */
while (iptr[1]) {               /* Skip trailing quote */
    if (*iptr != '\\') {
        if (*iptr == quote_char)
            return SCPE_ARG;    /* Imbedded quotes must be escaped */
        *(optr++) = (uint8)(*(iptr++));
        continue;
        }
    ++iptr; /* Skip backslash */
    switch (*iptr) {
        case 'r':   /* ASCII Carriage Return character (Decimal value 13) */
            *(optr++) = 13; ++iptr;
            break;
        case 'n':   /* ASCII Linefeed character (Decimal value 10) */
            *(optr++) = 10; ++iptr;
            break;
        case 'f':   /* ASCII Formfeed character (Decimal value 12) */
            *(optr++) = 12; ++iptr;
            break;
        case 't':   /* ASCII Horizontal Tab character (Decimal value 9) */
            *(optr++) = 9; ++iptr;
            break;
        case 'v':   /* ASCII Vertical Tab character (Decimal value 11) */
            *(optr++) = 11; ++iptr;
            break;
        case 'b':   /* ASCII Backspace character (Decimal value 8) */
            *(optr++) = 8; ++iptr;
            break;
        case '\\':   /* ASCII Backslash character (Decimal value 92) */
            *(optr++) = 92; ++iptr;
            break;
        case 'e':   /* ASCII Escape character (Decimal value 27) */
            *(optr++) = 27; ++iptr;
            break;
        case '\'':   /* ASCII Single Quote character (Decimal value 39) */
            *(optr++) = 39; ++iptr;
            break;
        case '"':   /* ASCII Double Quote character (Decimal value 34) */
            *(optr++) = 34; ++iptr;
            break;
        case '?':   /* ASCII Question Mark character (Decimal value 63) */
            *(optr++) = 63; ++iptr;
            break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            *optr = *(iptr++) - '0';
            if ((*iptr >= '0') && (*iptr <= '7'))
                *optr = ((*optr)<<3) + (*(iptr++) - '0');
            if ((*iptr >= '0') && (*iptr <= '7'))
                *optr = ((*optr)<<3) + (*(iptr++) - '0');
            ++optr;
            break;
        case 'x':
            if (1) {
                static const char *hex_digits = "0123456789ABCDEF";
                const char *c;

                ++iptr;
                *optr = 0;
                c = strchr (hex_digits, sim_toupper(*iptr));
                if (c) {
                    *optr = ((*optr)<<4) + (uint8)(c-hex_digits);
                    ++iptr;
                    }
                c = strchr (hex_digits, sim_toupper(*iptr));
                if (c) {
                    *optr = ((*optr)<<4) + (uint8)(c-hex_digits);
                    ++iptr;
                    }
                ++optr;
                }
            break;
        default:
            return SCPE_ARG;    /* Invalid escape */
        }
    }
*optr = '\0';
*osize = (uint32)(optr-ostart);
return SCPE_OK;
}

int32 sim_int_char = 005;   // ^E
t_stat sim_poll_kbd (void) {
    // TODO: signals? SCPE_STOP, SCPE_BREAK
    // SCPE_KFLAG | ch
    return SCPE_OK;
}

int main(void)
{
    sim_reset_all();   /* power-on reset Altair */

    while (1) {
        t_stat r = sim_instr();
        if (r != SCPE_OK)
            break;
    }

    return 0;
}
