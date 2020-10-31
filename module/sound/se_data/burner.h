#ifndef INCLUDED_BURNER_H
#define INCLUDED_BURNER_H

u_char burner00[] = {
	0x00,0x00,0x7E,0xD0,0x00,0x00,0xB4,0xD5,
	0x00,0x00,0x00,0xF7,0x00,0x00,0x28,0xD2,
	0x0C,0x0D,0x53,0xD7,0x00,0x00,0x3C,0xD8,
	0x00,0x00,0x0F,0xD9,0x56,0x63,0x69,0x35,
	0x32,0x50,0x00,0xE4,0x00,0x00,0xFE,0xFF
};

#if !defined(BORMAN_DEMO)
u_char burner01[] = {
#if defined(PAL_RELEASE)
	0x00,0x01,0x7E,0xD0,0x00,0x00,0xB4,0xD5,
	0x00,0x01,0x01,0xFE,0x00,0x00,0x23,0xF6,
	0x00,0x00,0x00,0xD2,0x0D,0x03,0x56,0xD7,
	0x00,0x00,0x3F,0xD8,0x00,0x00,0x10,0xD9,
	0x01,0x10,0x00,0xE5,0x0E,0x24,0x90,0x47,
	0x00,0x00,0xFE,0xFF
#else
	0x00,0x01,0x7E,0xD0,0x00,0x00,0xB4,0xD5,
	0x00,0x01,0x01,0xFE,0x00,0x00,0x23,0xF6,
	0x00,0x00,0x00,0xD2,0x0E,0x03,0x56,0xD7,
	0x00,0x00,0x00,0xD8,0x00,0x00,0x11,0xD9,
	0x01,0x10,0x00,0xE5,0x0F,0x1E,0x60,0x47,
	0x00,0x00,0xFE,0xFF
#endif
};

u_char burner02[] = {
#if defined(PAL_RELEASE)
	0x00,0x00,0x7E,0xD0,0x00,0x00,0xB4,0xD5,
	0x00,0x01,0x01,0xFE,0x00,0x00,0x23,0xF6,
	0x00,0x00,0x0C,0xD2,0x0E,0x02,0x58,0xD7,
	0x00,0x00,0x3F,0xD8,0x00,0x00,0x10,0xD9,
	0x01,0x10,0x00,0xE5,0x11,0x24,0x90,0x33,
	0x00,0x00,0xFE,0xFF
#else
	0x00,0x00,0x7E,0xD0,0x00,0x00,0xB4,0xD5,
	0x00,0x01,0x01,0xFE,0x00,0x00,0x23,0xF6,
	0x00,0x00,0x0C,0xD2,0x0E,0x02,0x58,0xD7,
	0x00,0x00,0x00,0xD8,0x00,0x00,0x11,0xD9,
	0x01,0x10,0x00,0xE5,0x0E,0x1E,0x60,0x33,
	0x00,0x00,0xFE,0xFF
#endif
};
#endif // !BORMAN_DEMO

#endif /* END OF FILE */
