#ifndef INCLUDED_SENTAK01_H
#define INCLUDED_SENTAK01_H

u_char sentak0100[] = {
#if defined(BORMAN_DEMO)
	0x00,0x01,0x7E,0xD0,0x00,0x00,0xDC,0xD5,
	0x01,0x00,0x2B,0xF6,0x00,0x00,0x0E,0xD2,
	0x04,0x08,0x7F,0xD7,0x00,0x00,0x00,0xD8,
	0x00,0x00,0x12,0xD9,0x76,0x14,0x03,0x2B,
	0x14,0x14,0x03,0x2B,0x00,0x00,0xFE,0xFF
#else
	0x00,0x01,0x7E,0xD0,0x00,0x00,0xC8,0xD5,
	0x01,0x00,0x2B,0xF6,0x00,0x00,0x08,0xD2,
	0x04,0x08,0x7F,0xD7,0x00,0x00,0x00,0xD8,
	0x00,0x00,0x16,0xD9,0x54,0x28,0x01,0x40,
	0x00,0x00,0x21,0xD2,0x04,0x08,0x7F,0xD7,
	0x00,0x00,0x00,0xD8,0x00,0x00,0x16,0xD9,
	0x3C,0x14,0x01,0x36,0x0A,0x14,0x02,0x36,
	0x00,0x00,0xFE,0xFF
#endif
};

#endif /* END OF FILE */
