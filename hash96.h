static uint32_t
nextpow2(uint32_t val)
{
	val--; // we want to return 2 for 2, 4 for 4 etc.
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

#define rot32(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
static void
mix96(uint32_t *a, uint32_t *b, uint32_t *c)
{
	*a -= *c;  *a ^= rot32(*c, 4);  *c += *b;
	*b -= *a;  *b ^= rot32(*a, 6);  *a += *c;
	*c -= *b;  *c ^= rot32(*b, 8);  *b += *a;
	*a -= *c;  *a ^= rot32(*c,16);  *c += *b;
	*b -= *a;  *b ^= rot32(*a,19);  *a += *c;
	*c -= *b;  *c ^= rot32(*b, 4);  *b += *a;
}

static uint32_t
final96(uint32_t a, uint32_t b, uint32_t c)
{
	c ^= b; c -= rot32(b,14);
	a ^= c; a -= rot32(c,11);
	b ^= a; b -= rot32(a,25);
	c ^= b; c -= rot32(b,16);
	a ^= c; a -= rot32(c,4);
	b ^= a; b -= rot32(a,14);
	c ^= b; c -= rot32(b,24);
	return c;
}

static int
cmp96(uint32_t *a, uint32_t *b)
{
	return (a[0]-b[0]) | (a[1]-b[1]) | (a[2]-b[2]);
}

static void
copy96(uint32_t *dst, uint32_t *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}
#undef rot32
