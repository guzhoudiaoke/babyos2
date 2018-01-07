
_zombie:     file format elf32-i386


Disassembly of section .text:

00000000 <main>:
#include "stat.h"
#include "user.h"

int
main(void)
{
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 e4 f0             	and    $0xfffffff0,%esp
   6:	83 ec 10             	sub    $0x10,%esp
  if(fork() > 0)
   9:	e8 76 02 00 00       	call   284 <fork>
   e:	85 c0                	test   %eax,%eax
  10:	7e 0c                	jle    1e <main+0x1e>
    sleep(5);  // Let child exit before parent.
  12:	c7 04 24 05 00 00 00 	movl   $0x5,(%esp)
  19:	e8 fe 02 00 00       	call   31c <sleep>
  exit();
  1e:	e8 69 02 00 00       	call   28c <exit>
  23:	90                   	nop

00000024 <stosb>:
               "cc");
}

static inline void
stosb(void *addr, int data, int cnt)
{
  24:	55                   	push   %ebp
  25:	89 e5                	mov    %esp,%ebp
  27:	57                   	push   %edi
  28:	53                   	push   %ebx
  asm volatile("cld; rep stosb" :
  29:	8b 4d 08             	mov    0x8(%ebp),%ecx
  2c:	8b 55 10             	mov    0x10(%ebp),%edx
  2f:	8b 45 0c             	mov    0xc(%ebp),%eax
  32:	89 cb                	mov    %ecx,%ebx
  34:	89 df                	mov    %ebx,%edi
  36:	89 d1                	mov    %edx,%ecx
  38:	fc                   	cld    
  39:	f3 aa                	rep stos %al,%es:(%edi)
  3b:	89 ca                	mov    %ecx,%edx
  3d:	89 fb                	mov    %edi,%ebx
  3f:	89 5d 08             	mov    %ebx,0x8(%ebp)
  42:	89 55 10             	mov    %edx,0x10(%ebp)
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}
  45:	5b                   	pop    %ebx
  46:	5f                   	pop    %edi
  47:	5d                   	pop    %ebp
  48:	c3                   	ret    

00000049 <strcpy>:
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, char *t)
{
  49:	55                   	push   %ebp
  4a:	89 e5                	mov    %esp,%ebp
  4c:	83 ec 10             	sub    $0x10,%esp
  char *os;

  os = s;
  4f:	8b 45 08             	mov    0x8(%ebp),%eax
  52:	89 45 fc             	mov    %eax,-0x4(%ebp)
  while((*s++ = *t++) != 0)
  55:	90                   	nop
  56:	8b 45 08             	mov    0x8(%ebp),%eax
  59:	8d 50 01             	lea    0x1(%eax),%edx
  5c:	89 55 08             	mov    %edx,0x8(%ebp)
  5f:	8b 55 0c             	mov    0xc(%ebp),%edx
  62:	8d 4a 01             	lea    0x1(%edx),%ecx
  65:	89 4d 0c             	mov    %ecx,0xc(%ebp)
  68:	0f b6 12             	movzbl (%edx),%edx
  6b:	88 10                	mov    %dl,(%eax)
  6d:	0f b6 00             	movzbl (%eax),%eax
  70:	84 c0                	test   %al,%al
  72:	75 e2                	jne    56 <strcpy+0xd>
    ;
  return os;
  74:	8b 45 fc             	mov    -0x4(%ebp),%eax
}
  77:	c9                   	leave  
  78:	c3                   	ret    

00000079 <strcmp>:

int
strcmp(const char *p, const char *q)
{
  79:	55                   	push   %ebp
  7a:	89 e5                	mov    %esp,%ebp
  while(*p && *p == *q)
  7c:	eb 08                	jmp    86 <strcmp+0xd>
    p++, q++;
  7e:	83 45 08 01          	addl   $0x1,0x8(%ebp)
  82:	83 45 0c 01          	addl   $0x1,0xc(%ebp)
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
  86:	8b 45 08             	mov    0x8(%ebp),%eax
  89:	0f b6 00             	movzbl (%eax),%eax
  8c:	84 c0                	test   %al,%al
  8e:	74 10                	je     a0 <strcmp+0x27>
  90:	8b 45 08             	mov    0x8(%ebp),%eax
  93:	0f b6 10             	movzbl (%eax),%edx
  96:	8b 45 0c             	mov    0xc(%ebp),%eax
  99:	0f b6 00             	movzbl (%eax),%eax
  9c:	38 c2                	cmp    %al,%dl
  9e:	74 de                	je     7e <strcmp+0x5>
    p++, q++;
  return (uchar)*p - (uchar)*q;
  a0:	8b 45 08             	mov    0x8(%ebp),%eax
  a3:	0f b6 00             	movzbl (%eax),%eax
  a6:	0f b6 d0             	movzbl %al,%edx
  a9:	8b 45 0c             	mov    0xc(%ebp),%eax
  ac:	0f b6 00             	movzbl (%eax),%eax
  af:	0f b6 c0             	movzbl %al,%eax
  b2:	29 c2                	sub    %eax,%edx
  b4:	89 d0                	mov    %edx,%eax
}
  b6:	5d                   	pop    %ebp
  b7:	c3                   	ret    

000000b8 <strlen>:

uint
strlen(char *s)
{
  b8:	55                   	push   %ebp
  b9:	89 e5                	mov    %esp,%ebp
  bb:	83 ec 10             	sub    $0x10,%esp
  int n;

  for(n = 0; s[n]; n++)
  be:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%ebp)
  c5:	eb 04                	jmp    cb <strlen+0x13>
  c7:	83 45 fc 01          	addl   $0x1,-0x4(%ebp)
  cb:	8b 55 fc             	mov    -0x4(%ebp),%edx
  ce:	8b 45 08             	mov    0x8(%ebp),%eax
  d1:	01 d0                	add    %edx,%eax
  d3:	0f b6 00             	movzbl (%eax),%eax
  d6:	84 c0                	test   %al,%al
  d8:	75 ed                	jne    c7 <strlen+0xf>
    ;
  return n;
  da:	8b 45 fc             	mov    -0x4(%ebp),%eax
}
  dd:	c9                   	leave  
  de:	c3                   	ret    

000000df <memset>:

void*
memset(void *dst, int c, uint n)
{
  df:	55                   	push   %ebp
  e0:	89 e5                	mov    %esp,%ebp
  e2:	83 ec 0c             	sub    $0xc,%esp
  stosb(dst, c, n);
  e5:	8b 45 10             	mov    0x10(%ebp),%eax
  e8:	89 44 24 08          	mov    %eax,0x8(%esp)
  ec:	8b 45 0c             	mov    0xc(%ebp),%eax
  ef:	89 44 24 04          	mov    %eax,0x4(%esp)
  f3:	8b 45 08             	mov    0x8(%ebp),%eax
  f6:	89 04 24             	mov    %eax,(%esp)
  f9:	e8 26 ff ff ff       	call   24 <stosb>
  return dst;
  fe:	8b 45 08             	mov    0x8(%ebp),%eax
}
 101:	c9                   	leave  
 102:	c3                   	ret    

00000103 <strchr>:

char*
strchr(const char *s, char c)
{
 103:	55                   	push   %ebp
 104:	89 e5                	mov    %esp,%ebp
 106:	83 ec 04             	sub    $0x4,%esp
 109:	8b 45 0c             	mov    0xc(%ebp),%eax
 10c:	88 45 fc             	mov    %al,-0x4(%ebp)
  for(; *s; s++)
 10f:	eb 14                	jmp    125 <strchr+0x22>
    if(*s == c)
 111:	8b 45 08             	mov    0x8(%ebp),%eax
 114:	0f b6 00             	movzbl (%eax),%eax
 117:	3a 45 fc             	cmp    -0x4(%ebp),%al
 11a:	75 05                	jne    121 <strchr+0x1e>
      return (char*)s;
 11c:	8b 45 08             	mov    0x8(%ebp),%eax
 11f:	eb 13                	jmp    134 <strchr+0x31>
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
 121:	83 45 08 01          	addl   $0x1,0x8(%ebp)
 125:	8b 45 08             	mov    0x8(%ebp),%eax
 128:	0f b6 00             	movzbl (%eax),%eax
 12b:	84 c0                	test   %al,%al
 12d:	75 e2                	jne    111 <strchr+0xe>
    if(*s == c)
      return (char*)s;
  return 0;
 12f:	b8 00 00 00 00       	mov    $0x0,%eax
}
 134:	c9                   	leave  
 135:	c3                   	ret    

00000136 <gets>:

char*
gets(char *buf, int max)
{
 136:	55                   	push   %ebp
 137:	89 e5                	mov    %esp,%ebp
 139:	83 ec 28             	sub    $0x28,%esp
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 13c:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
 143:	eb 4c                	jmp    191 <gets+0x5b>
    cc = read(0, &c, 1);
 145:	c7 44 24 08 01 00 00 	movl   $0x1,0x8(%esp)
 14c:	00 
 14d:	8d 45 ef             	lea    -0x11(%ebp),%eax
 150:	89 44 24 04          	mov    %eax,0x4(%esp)
 154:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
 15b:	e8 44 01 00 00       	call   2a4 <read>
 160:	89 45 f0             	mov    %eax,-0x10(%ebp)
    if(cc < 1)
 163:	83 7d f0 00          	cmpl   $0x0,-0x10(%ebp)
 167:	7f 02                	jg     16b <gets+0x35>
      break;
 169:	eb 31                	jmp    19c <gets+0x66>
    buf[i++] = c;
 16b:	8b 45 f4             	mov    -0xc(%ebp),%eax
 16e:	8d 50 01             	lea    0x1(%eax),%edx
 171:	89 55 f4             	mov    %edx,-0xc(%ebp)
 174:	89 c2                	mov    %eax,%edx
 176:	8b 45 08             	mov    0x8(%ebp),%eax
 179:	01 c2                	add    %eax,%edx
 17b:	0f b6 45 ef          	movzbl -0x11(%ebp),%eax
 17f:	88 02                	mov    %al,(%edx)
    if(c == '\n' || c == '\r')
 181:	0f b6 45 ef          	movzbl -0x11(%ebp),%eax
 185:	3c 0a                	cmp    $0xa,%al
 187:	74 13                	je     19c <gets+0x66>
 189:	0f b6 45 ef          	movzbl -0x11(%ebp),%eax
 18d:	3c 0d                	cmp    $0xd,%al
 18f:	74 0b                	je     19c <gets+0x66>
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 191:	8b 45 f4             	mov    -0xc(%ebp),%eax
 194:	83 c0 01             	add    $0x1,%eax
 197:	3b 45 0c             	cmp    0xc(%ebp),%eax
 19a:	7c a9                	jl     145 <gets+0xf>
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
 19c:	8b 55 f4             	mov    -0xc(%ebp),%edx
 19f:	8b 45 08             	mov    0x8(%ebp),%eax
 1a2:	01 d0                	add    %edx,%eax
 1a4:	c6 00 00             	movb   $0x0,(%eax)
  return buf;
 1a7:	8b 45 08             	mov    0x8(%ebp),%eax
}
 1aa:	c9                   	leave  
 1ab:	c3                   	ret    

000001ac <stat>:

int
stat(char *n, struct stat *st)
{
 1ac:	55                   	push   %ebp
 1ad:	89 e5                	mov    %esp,%ebp
 1af:	83 ec 28             	sub    $0x28,%esp
  int fd;
  int r;

  fd = open(n, O_RDONLY);
 1b2:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
 1b9:	00 
 1ba:	8b 45 08             	mov    0x8(%ebp),%eax
 1bd:	89 04 24             	mov    %eax,(%esp)
 1c0:	e8 07 01 00 00       	call   2cc <open>
 1c5:	89 45 f4             	mov    %eax,-0xc(%ebp)
  if(fd < 0)
 1c8:	83 7d f4 00          	cmpl   $0x0,-0xc(%ebp)
 1cc:	79 07                	jns    1d5 <stat+0x29>
    return -1;
 1ce:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
 1d3:	eb 23                	jmp    1f8 <stat+0x4c>
  r = fstat(fd, st);
 1d5:	8b 45 0c             	mov    0xc(%ebp),%eax
 1d8:	89 44 24 04          	mov    %eax,0x4(%esp)
 1dc:	8b 45 f4             	mov    -0xc(%ebp),%eax
 1df:	89 04 24             	mov    %eax,(%esp)
 1e2:	e8 fd 00 00 00       	call   2e4 <fstat>
 1e7:	89 45 f0             	mov    %eax,-0x10(%ebp)
  close(fd);
 1ea:	8b 45 f4             	mov    -0xc(%ebp),%eax
 1ed:	89 04 24             	mov    %eax,(%esp)
 1f0:	e8 bf 00 00 00       	call   2b4 <close>
  return r;
 1f5:	8b 45 f0             	mov    -0x10(%ebp),%eax
}
 1f8:	c9                   	leave  
 1f9:	c3                   	ret    

000001fa <atoi>:

int
atoi(const char *s)
{
 1fa:	55                   	push   %ebp
 1fb:	89 e5                	mov    %esp,%ebp
 1fd:	83 ec 10             	sub    $0x10,%esp
  int n;

  n = 0;
 200:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%ebp)
  while('0' <= *s && *s <= '9')
 207:	eb 25                	jmp    22e <atoi+0x34>
    n = n*10 + *s++ - '0';
 209:	8b 55 fc             	mov    -0x4(%ebp),%edx
 20c:	89 d0                	mov    %edx,%eax
 20e:	c1 e0 02             	shl    $0x2,%eax
 211:	01 d0                	add    %edx,%eax
 213:	01 c0                	add    %eax,%eax
 215:	89 c1                	mov    %eax,%ecx
 217:	8b 45 08             	mov    0x8(%ebp),%eax
 21a:	8d 50 01             	lea    0x1(%eax),%edx
 21d:	89 55 08             	mov    %edx,0x8(%ebp)
 220:	0f b6 00             	movzbl (%eax),%eax
 223:	0f be c0             	movsbl %al,%eax
 226:	01 c8                	add    %ecx,%eax
 228:	83 e8 30             	sub    $0x30,%eax
 22b:	89 45 fc             	mov    %eax,-0x4(%ebp)
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
 22e:	8b 45 08             	mov    0x8(%ebp),%eax
 231:	0f b6 00             	movzbl (%eax),%eax
 234:	3c 2f                	cmp    $0x2f,%al
 236:	7e 0a                	jle    242 <atoi+0x48>
 238:	8b 45 08             	mov    0x8(%ebp),%eax
 23b:	0f b6 00             	movzbl (%eax),%eax
 23e:	3c 39                	cmp    $0x39,%al
 240:	7e c7                	jle    209 <atoi+0xf>
    n = n*10 + *s++ - '0';
  return n;
 242:	8b 45 fc             	mov    -0x4(%ebp),%eax
}
 245:	c9                   	leave  
 246:	c3                   	ret    

00000247 <memmove>:

void*
memmove(void *vdst, void *vsrc, int n)
{
 247:	55                   	push   %ebp
 248:	89 e5                	mov    %esp,%ebp
 24a:	83 ec 10             	sub    $0x10,%esp
  char *dst, *src;
  
  dst = vdst;
 24d:	8b 45 08             	mov    0x8(%ebp),%eax
 250:	89 45 fc             	mov    %eax,-0x4(%ebp)
  src = vsrc;
 253:	8b 45 0c             	mov    0xc(%ebp),%eax
 256:	89 45 f8             	mov    %eax,-0x8(%ebp)
  while(n-- > 0)
 259:	eb 17                	jmp    272 <memmove+0x2b>
    *dst++ = *src++;
 25b:	8b 45 fc             	mov    -0x4(%ebp),%eax
 25e:	8d 50 01             	lea    0x1(%eax),%edx
 261:	89 55 fc             	mov    %edx,-0x4(%ebp)
 264:	8b 55 f8             	mov    -0x8(%ebp),%edx
 267:	8d 4a 01             	lea    0x1(%edx),%ecx
 26a:	89 4d f8             	mov    %ecx,-0x8(%ebp)
 26d:	0f b6 12             	movzbl (%edx),%edx
 270:	88 10                	mov    %dl,(%eax)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
 272:	8b 45 10             	mov    0x10(%ebp),%eax
 275:	8d 50 ff             	lea    -0x1(%eax),%edx
 278:	89 55 10             	mov    %edx,0x10(%ebp)
 27b:	85 c0                	test   %eax,%eax
 27d:	7f dc                	jg     25b <memmove+0x14>
    *dst++ = *src++;
  return vdst;
 27f:	8b 45 08             	mov    0x8(%ebp),%eax
}
 282:	c9                   	leave  
 283:	c3                   	ret    

00000284 <fork>:
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
 284:	b8 01 00 00 00       	mov    $0x1,%eax
 289:	cd 40                	int    $0x40
 28b:	c3                   	ret    

0000028c <exit>:
SYSCALL(exit)
 28c:	b8 02 00 00 00       	mov    $0x2,%eax
 291:	cd 40                	int    $0x40
 293:	c3                   	ret    

00000294 <wait>:
SYSCALL(wait)
 294:	b8 03 00 00 00       	mov    $0x3,%eax
 299:	cd 40                	int    $0x40
 29b:	c3                   	ret    

0000029c <pipe>:
SYSCALL(pipe)
 29c:	b8 04 00 00 00       	mov    $0x4,%eax
 2a1:	cd 40                	int    $0x40
 2a3:	c3                   	ret    

000002a4 <read>:
SYSCALL(read)
 2a4:	b8 05 00 00 00       	mov    $0x5,%eax
 2a9:	cd 40                	int    $0x40
 2ab:	c3                   	ret    

000002ac <write>:
SYSCALL(write)
 2ac:	b8 10 00 00 00       	mov    $0x10,%eax
 2b1:	cd 40                	int    $0x40
 2b3:	c3                   	ret    

000002b4 <close>:
SYSCALL(close)
 2b4:	b8 15 00 00 00       	mov    $0x15,%eax
 2b9:	cd 40                	int    $0x40
 2bb:	c3                   	ret    

000002bc <kill>:
SYSCALL(kill)
 2bc:	b8 06 00 00 00       	mov    $0x6,%eax
 2c1:	cd 40                	int    $0x40
 2c3:	c3                   	ret    

000002c4 <exec>:
SYSCALL(exec)
 2c4:	b8 07 00 00 00       	mov    $0x7,%eax
 2c9:	cd 40                	int    $0x40
 2cb:	c3                   	ret    

000002cc <open>:
SYSCALL(open)
 2cc:	b8 0f 00 00 00       	mov    $0xf,%eax
 2d1:	cd 40                	int    $0x40
 2d3:	c3                   	ret    

000002d4 <mknod>:
SYSCALL(mknod)
 2d4:	b8 11 00 00 00       	mov    $0x11,%eax
 2d9:	cd 40                	int    $0x40
 2db:	c3                   	ret    

000002dc <unlink>:
SYSCALL(unlink)
 2dc:	b8 12 00 00 00       	mov    $0x12,%eax
 2e1:	cd 40                	int    $0x40
 2e3:	c3                   	ret    

000002e4 <fstat>:
SYSCALL(fstat)
 2e4:	b8 08 00 00 00       	mov    $0x8,%eax
 2e9:	cd 40                	int    $0x40
 2eb:	c3                   	ret    

000002ec <link>:
SYSCALL(link)
 2ec:	b8 13 00 00 00       	mov    $0x13,%eax
 2f1:	cd 40                	int    $0x40
 2f3:	c3                   	ret    

000002f4 <mkdir>:
SYSCALL(mkdir)
 2f4:	b8 14 00 00 00       	mov    $0x14,%eax
 2f9:	cd 40                	int    $0x40
 2fb:	c3                   	ret    

000002fc <chdir>:
SYSCALL(chdir)
 2fc:	b8 09 00 00 00       	mov    $0x9,%eax
 301:	cd 40                	int    $0x40
 303:	c3                   	ret    

00000304 <dup>:
SYSCALL(dup)
 304:	b8 0a 00 00 00       	mov    $0xa,%eax
 309:	cd 40                	int    $0x40
 30b:	c3                   	ret    

0000030c <getpid>:
SYSCALL(getpid)
 30c:	b8 0b 00 00 00       	mov    $0xb,%eax
 311:	cd 40                	int    $0x40
 313:	c3                   	ret    

00000314 <sbrk>:
SYSCALL(sbrk)
 314:	b8 0c 00 00 00       	mov    $0xc,%eax
 319:	cd 40                	int    $0x40
 31b:	c3                   	ret    

0000031c <sleep>:
SYSCALL(sleep)
 31c:	b8 0d 00 00 00       	mov    $0xd,%eax
 321:	cd 40                	int    $0x40
 323:	c3                   	ret    

00000324 <uptime>:
SYSCALL(uptime)
 324:	b8 0e 00 00 00       	mov    $0xe,%eax
 329:	cd 40                	int    $0x40
 32b:	c3                   	ret    

0000032c <putc>:
#include "stat.h"
#include "user.h"

static void
putc(int fd, char c)
{
 32c:	55                   	push   %ebp
 32d:	89 e5                	mov    %esp,%ebp
 32f:	83 ec 18             	sub    $0x18,%esp
 332:	8b 45 0c             	mov    0xc(%ebp),%eax
 335:	88 45 f4             	mov    %al,-0xc(%ebp)
  write(fd, &c, 1);
 338:	c7 44 24 08 01 00 00 	movl   $0x1,0x8(%esp)
 33f:	00 
 340:	8d 45 f4             	lea    -0xc(%ebp),%eax
 343:	89 44 24 04          	mov    %eax,0x4(%esp)
 347:	8b 45 08             	mov    0x8(%ebp),%eax
 34a:	89 04 24             	mov    %eax,(%esp)
 34d:	e8 5a ff ff ff       	call   2ac <write>
}
 352:	c9                   	leave  
 353:	c3                   	ret    

00000354 <printint>:

static void
printint(int fd, int xx, int base, int sgn)
{
 354:	55                   	push   %ebp
 355:	89 e5                	mov    %esp,%ebp
 357:	56                   	push   %esi
 358:	53                   	push   %ebx
 359:	83 ec 30             	sub    $0x30,%esp
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
 35c:	c7 45 f0 00 00 00 00 	movl   $0x0,-0x10(%ebp)
  if(sgn && xx < 0){
 363:	83 7d 14 00          	cmpl   $0x0,0x14(%ebp)
 367:	74 17                	je     380 <printint+0x2c>
 369:	83 7d 0c 00          	cmpl   $0x0,0xc(%ebp)
 36d:	79 11                	jns    380 <printint+0x2c>
    neg = 1;
 36f:	c7 45 f0 01 00 00 00 	movl   $0x1,-0x10(%ebp)
    x = -xx;
 376:	8b 45 0c             	mov    0xc(%ebp),%eax
 379:	f7 d8                	neg    %eax
 37b:	89 45 ec             	mov    %eax,-0x14(%ebp)
 37e:	eb 06                	jmp    386 <printint+0x32>
  } else {
    x = xx;
 380:	8b 45 0c             	mov    0xc(%ebp),%eax
 383:	89 45 ec             	mov    %eax,-0x14(%ebp)
  }

  i = 0;
 386:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
  do{
    buf[i++] = digits[x % base];
 38d:	8b 4d f4             	mov    -0xc(%ebp),%ecx
 390:	8d 41 01             	lea    0x1(%ecx),%eax
 393:	89 45 f4             	mov    %eax,-0xc(%ebp)
 396:	8b 5d 10             	mov    0x10(%ebp),%ebx
 399:	8b 45 ec             	mov    -0x14(%ebp),%eax
 39c:	ba 00 00 00 00       	mov    $0x0,%edx
 3a1:	f7 f3                	div    %ebx
 3a3:	89 d0                	mov    %edx,%eax
 3a5:	0f b6 80 24 0a 00 00 	movzbl 0xa24(%eax),%eax
 3ac:	88 44 0d dc          	mov    %al,-0x24(%ebp,%ecx,1)
  }while((x /= base) != 0);
 3b0:	8b 75 10             	mov    0x10(%ebp),%esi
 3b3:	8b 45 ec             	mov    -0x14(%ebp),%eax
 3b6:	ba 00 00 00 00       	mov    $0x0,%edx
 3bb:	f7 f6                	div    %esi
 3bd:	89 45 ec             	mov    %eax,-0x14(%ebp)
 3c0:	83 7d ec 00          	cmpl   $0x0,-0x14(%ebp)
 3c4:	75 c7                	jne    38d <printint+0x39>
  if(neg)
 3c6:	83 7d f0 00          	cmpl   $0x0,-0x10(%ebp)
 3ca:	74 10                	je     3dc <printint+0x88>
    buf[i++] = '-';
 3cc:	8b 45 f4             	mov    -0xc(%ebp),%eax
 3cf:	8d 50 01             	lea    0x1(%eax),%edx
 3d2:	89 55 f4             	mov    %edx,-0xc(%ebp)
 3d5:	c6 44 05 dc 2d       	movb   $0x2d,-0x24(%ebp,%eax,1)

  while(--i >= 0)
 3da:	eb 1f                	jmp    3fb <printint+0xa7>
 3dc:	eb 1d                	jmp    3fb <printint+0xa7>
    putc(fd, buf[i]);
 3de:	8d 55 dc             	lea    -0x24(%ebp),%edx
 3e1:	8b 45 f4             	mov    -0xc(%ebp),%eax
 3e4:	01 d0                	add    %edx,%eax
 3e6:	0f b6 00             	movzbl (%eax),%eax
 3e9:	0f be c0             	movsbl %al,%eax
 3ec:	89 44 24 04          	mov    %eax,0x4(%esp)
 3f0:	8b 45 08             	mov    0x8(%ebp),%eax
 3f3:	89 04 24             	mov    %eax,(%esp)
 3f6:	e8 31 ff ff ff       	call   32c <putc>
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
 3fb:	83 6d f4 01          	subl   $0x1,-0xc(%ebp)
 3ff:	83 7d f4 00          	cmpl   $0x0,-0xc(%ebp)
 403:	79 d9                	jns    3de <printint+0x8a>
    putc(fd, buf[i]);
}
 405:	83 c4 30             	add    $0x30,%esp
 408:	5b                   	pop    %ebx
 409:	5e                   	pop    %esi
 40a:	5d                   	pop    %ebp
 40b:	c3                   	ret    

0000040c <printf>:

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, char *fmt, ...)
{
 40c:	55                   	push   %ebp
 40d:	89 e5                	mov    %esp,%ebp
 40f:	83 ec 38             	sub    $0x38,%esp
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
 412:	c7 45 ec 00 00 00 00 	movl   $0x0,-0x14(%ebp)
  ap = (uint*)(void*)&fmt + 1;
 419:	8d 45 0c             	lea    0xc(%ebp),%eax
 41c:	83 c0 04             	add    $0x4,%eax
 41f:	89 45 e8             	mov    %eax,-0x18(%ebp)
  for(i = 0; fmt[i]; i++){
 422:	c7 45 f0 00 00 00 00 	movl   $0x0,-0x10(%ebp)
 429:	e9 7c 01 00 00       	jmp    5aa <printf+0x19e>
    c = fmt[i] & 0xff;
 42e:	8b 55 0c             	mov    0xc(%ebp),%edx
 431:	8b 45 f0             	mov    -0x10(%ebp),%eax
 434:	01 d0                	add    %edx,%eax
 436:	0f b6 00             	movzbl (%eax),%eax
 439:	0f be c0             	movsbl %al,%eax
 43c:	25 ff 00 00 00       	and    $0xff,%eax
 441:	89 45 e4             	mov    %eax,-0x1c(%ebp)
    if(state == 0){
 444:	83 7d ec 00          	cmpl   $0x0,-0x14(%ebp)
 448:	75 2c                	jne    476 <printf+0x6a>
      if(c == '%'){
 44a:	83 7d e4 25          	cmpl   $0x25,-0x1c(%ebp)
 44e:	75 0c                	jne    45c <printf+0x50>
        state = '%';
 450:	c7 45 ec 25 00 00 00 	movl   $0x25,-0x14(%ebp)
 457:	e9 4a 01 00 00       	jmp    5a6 <printf+0x19a>
      } else {
        putc(fd, c);
 45c:	8b 45 e4             	mov    -0x1c(%ebp),%eax
 45f:	0f be c0             	movsbl %al,%eax
 462:	89 44 24 04          	mov    %eax,0x4(%esp)
 466:	8b 45 08             	mov    0x8(%ebp),%eax
 469:	89 04 24             	mov    %eax,(%esp)
 46c:	e8 bb fe ff ff       	call   32c <putc>
 471:	e9 30 01 00 00       	jmp    5a6 <printf+0x19a>
      }
    } else if(state == '%'){
 476:	83 7d ec 25          	cmpl   $0x25,-0x14(%ebp)
 47a:	0f 85 26 01 00 00    	jne    5a6 <printf+0x19a>
      if(c == 'd'){
 480:	83 7d e4 64          	cmpl   $0x64,-0x1c(%ebp)
 484:	75 2d                	jne    4b3 <printf+0xa7>
        printint(fd, *ap, 10, 1);
 486:	8b 45 e8             	mov    -0x18(%ebp),%eax
 489:	8b 00                	mov    (%eax),%eax
 48b:	c7 44 24 0c 01 00 00 	movl   $0x1,0xc(%esp)
 492:	00 
 493:	c7 44 24 08 0a 00 00 	movl   $0xa,0x8(%esp)
 49a:	00 
 49b:	89 44 24 04          	mov    %eax,0x4(%esp)
 49f:	8b 45 08             	mov    0x8(%ebp),%eax
 4a2:	89 04 24             	mov    %eax,(%esp)
 4a5:	e8 aa fe ff ff       	call   354 <printint>
        ap++;
 4aa:	83 45 e8 04          	addl   $0x4,-0x18(%ebp)
 4ae:	e9 ec 00 00 00       	jmp    59f <printf+0x193>
      } else if(c == 'x' || c == 'p'){
 4b3:	83 7d e4 78          	cmpl   $0x78,-0x1c(%ebp)
 4b7:	74 06                	je     4bf <printf+0xb3>
 4b9:	83 7d e4 70          	cmpl   $0x70,-0x1c(%ebp)
 4bd:	75 2d                	jne    4ec <printf+0xe0>
        printint(fd, *ap, 16, 0);
 4bf:	8b 45 e8             	mov    -0x18(%ebp),%eax
 4c2:	8b 00                	mov    (%eax),%eax
 4c4:	c7 44 24 0c 00 00 00 	movl   $0x0,0xc(%esp)
 4cb:	00 
 4cc:	c7 44 24 08 10 00 00 	movl   $0x10,0x8(%esp)
 4d3:	00 
 4d4:	89 44 24 04          	mov    %eax,0x4(%esp)
 4d8:	8b 45 08             	mov    0x8(%ebp),%eax
 4db:	89 04 24             	mov    %eax,(%esp)
 4de:	e8 71 fe ff ff       	call   354 <printint>
        ap++;
 4e3:	83 45 e8 04          	addl   $0x4,-0x18(%ebp)
 4e7:	e9 b3 00 00 00       	jmp    59f <printf+0x193>
      } else if(c == 's'){
 4ec:	83 7d e4 73          	cmpl   $0x73,-0x1c(%ebp)
 4f0:	75 45                	jne    537 <printf+0x12b>
        s = (char*)*ap;
 4f2:	8b 45 e8             	mov    -0x18(%ebp),%eax
 4f5:	8b 00                	mov    (%eax),%eax
 4f7:	89 45 f4             	mov    %eax,-0xc(%ebp)
        ap++;
 4fa:	83 45 e8 04          	addl   $0x4,-0x18(%ebp)
        if(s == 0)
 4fe:	83 7d f4 00          	cmpl   $0x0,-0xc(%ebp)
 502:	75 09                	jne    50d <printf+0x101>
          s = "(null)";
 504:	c7 45 f4 d9 07 00 00 	movl   $0x7d9,-0xc(%ebp)
        while(*s != 0){
 50b:	eb 1e                	jmp    52b <printf+0x11f>
 50d:	eb 1c                	jmp    52b <printf+0x11f>
          putc(fd, *s);
 50f:	8b 45 f4             	mov    -0xc(%ebp),%eax
 512:	0f b6 00             	movzbl (%eax),%eax
 515:	0f be c0             	movsbl %al,%eax
 518:	89 44 24 04          	mov    %eax,0x4(%esp)
 51c:	8b 45 08             	mov    0x8(%ebp),%eax
 51f:	89 04 24             	mov    %eax,(%esp)
 522:	e8 05 fe ff ff       	call   32c <putc>
          s++;
 527:	83 45 f4 01          	addl   $0x1,-0xc(%ebp)
      } else if(c == 's'){
        s = (char*)*ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
 52b:	8b 45 f4             	mov    -0xc(%ebp),%eax
 52e:	0f b6 00             	movzbl (%eax),%eax
 531:	84 c0                	test   %al,%al
 533:	75 da                	jne    50f <printf+0x103>
 535:	eb 68                	jmp    59f <printf+0x193>
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c'){
 537:	83 7d e4 63          	cmpl   $0x63,-0x1c(%ebp)
 53b:	75 1d                	jne    55a <printf+0x14e>
        putc(fd, *ap);
 53d:	8b 45 e8             	mov    -0x18(%ebp),%eax
 540:	8b 00                	mov    (%eax),%eax
 542:	0f be c0             	movsbl %al,%eax
 545:	89 44 24 04          	mov    %eax,0x4(%esp)
 549:	8b 45 08             	mov    0x8(%ebp),%eax
 54c:	89 04 24             	mov    %eax,(%esp)
 54f:	e8 d8 fd ff ff       	call   32c <putc>
        ap++;
 554:	83 45 e8 04          	addl   $0x4,-0x18(%ebp)
 558:	eb 45                	jmp    59f <printf+0x193>
      } else if(c == '%'){
 55a:	83 7d e4 25          	cmpl   $0x25,-0x1c(%ebp)
 55e:	75 17                	jne    577 <printf+0x16b>
        putc(fd, c);
 560:	8b 45 e4             	mov    -0x1c(%ebp),%eax
 563:	0f be c0             	movsbl %al,%eax
 566:	89 44 24 04          	mov    %eax,0x4(%esp)
 56a:	8b 45 08             	mov    0x8(%ebp),%eax
 56d:	89 04 24             	mov    %eax,(%esp)
 570:	e8 b7 fd ff ff       	call   32c <putc>
 575:	eb 28                	jmp    59f <printf+0x193>
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
 577:	c7 44 24 04 25 00 00 	movl   $0x25,0x4(%esp)
 57e:	00 
 57f:	8b 45 08             	mov    0x8(%ebp),%eax
 582:	89 04 24             	mov    %eax,(%esp)
 585:	e8 a2 fd ff ff       	call   32c <putc>
        putc(fd, c);
 58a:	8b 45 e4             	mov    -0x1c(%ebp),%eax
 58d:	0f be c0             	movsbl %al,%eax
 590:	89 44 24 04          	mov    %eax,0x4(%esp)
 594:	8b 45 08             	mov    0x8(%ebp),%eax
 597:	89 04 24             	mov    %eax,(%esp)
 59a:	e8 8d fd ff ff       	call   32c <putc>
      }
      state = 0;
 59f:	c7 45 ec 00 00 00 00 	movl   $0x0,-0x14(%ebp)
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
 5a6:	83 45 f0 01          	addl   $0x1,-0x10(%ebp)
 5aa:	8b 55 0c             	mov    0xc(%ebp),%edx
 5ad:	8b 45 f0             	mov    -0x10(%ebp),%eax
 5b0:	01 d0                	add    %edx,%eax
 5b2:	0f b6 00             	movzbl (%eax),%eax
 5b5:	84 c0                	test   %al,%al
 5b7:	0f 85 71 fe ff ff    	jne    42e <printf+0x22>
        putc(fd, c);
      }
      state = 0;
    }
  }
}
 5bd:	c9                   	leave  
 5be:	c3                   	ret    
 5bf:	90                   	nop

000005c0 <free>:
static Header base;
static Header *freep;

void
free(void *ap)
{
 5c0:	55                   	push   %ebp
 5c1:	89 e5                	mov    %esp,%ebp
 5c3:	83 ec 10             	sub    $0x10,%esp
  Header *bp, *p;

  bp = (Header*)ap - 1;
 5c6:	8b 45 08             	mov    0x8(%ebp),%eax
 5c9:	83 e8 08             	sub    $0x8,%eax
 5cc:	89 45 f8             	mov    %eax,-0x8(%ebp)
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 5cf:	a1 40 0a 00 00       	mov    0xa40,%eax
 5d4:	89 45 fc             	mov    %eax,-0x4(%ebp)
 5d7:	eb 24                	jmp    5fd <free+0x3d>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 5d9:	8b 45 fc             	mov    -0x4(%ebp),%eax
 5dc:	8b 00                	mov    (%eax),%eax
 5de:	3b 45 fc             	cmp    -0x4(%ebp),%eax
 5e1:	77 12                	ja     5f5 <free+0x35>
 5e3:	8b 45 f8             	mov    -0x8(%ebp),%eax
 5e6:	3b 45 fc             	cmp    -0x4(%ebp),%eax
 5e9:	77 24                	ja     60f <free+0x4f>
 5eb:	8b 45 fc             	mov    -0x4(%ebp),%eax
 5ee:	8b 00                	mov    (%eax),%eax
 5f0:	3b 45 f8             	cmp    -0x8(%ebp),%eax
 5f3:	77 1a                	ja     60f <free+0x4f>
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 5f5:	8b 45 fc             	mov    -0x4(%ebp),%eax
 5f8:	8b 00                	mov    (%eax),%eax
 5fa:	89 45 fc             	mov    %eax,-0x4(%ebp)
 5fd:	8b 45 f8             	mov    -0x8(%ebp),%eax
 600:	3b 45 fc             	cmp    -0x4(%ebp),%eax
 603:	76 d4                	jbe    5d9 <free+0x19>
 605:	8b 45 fc             	mov    -0x4(%ebp),%eax
 608:	8b 00                	mov    (%eax),%eax
 60a:	3b 45 f8             	cmp    -0x8(%ebp),%eax
 60d:	76 ca                	jbe    5d9 <free+0x19>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
 60f:	8b 45 f8             	mov    -0x8(%ebp),%eax
 612:	8b 40 04             	mov    0x4(%eax),%eax
 615:	8d 14 c5 00 00 00 00 	lea    0x0(,%eax,8),%edx
 61c:	8b 45 f8             	mov    -0x8(%ebp),%eax
 61f:	01 c2                	add    %eax,%edx
 621:	8b 45 fc             	mov    -0x4(%ebp),%eax
 624:	8b 00                	mov    (%eax),%eax
 626:	39 c2                	cmp    %eax,%edx
 628:	75 24                	jne    64e <free+0x8e>
    bp->s.size += p->s.ptr->s.size;
 62a:	8b 45 f8             	mov    -0x8(%ebp),%eax
 62d:	8b 50 04             	mov    0x4(%eax),%edx
 630:	8b 45 fc             	mov    -0x4(%ebp),%eax
 633:	8b 00                	mov    (%eax),%eax
 635:	8b 40 04             	mov    0x4(%eax),%eax
 638:	01 c2                	add    %eax,%edx
 63a:	8b 45 f8             	mov    -0x8(%ebp),%eax
 63d:	89 50 04             	mov    %edx,0x4(%eax)
    bp->s.ptr = p->s.ptr->s.ptr;
 640:	8b 45 fc             	mov    -0x4(%ebp),%eax
 643:	8b 00                	mov    (%eax),%eax
 645:	8b 10                	mov    (%eax),%edx
 647:	8b 45 f8             	mov    -0x8(%ebp),%eax
 64a:	89 10                	mov    %edx,(%eax)
 64c:	eb 0a                	jmp    658 <free+0x98>
  } else
    bp->s.ptr = p->s.ptr;
 64e:	8b 45 fc             	mov    -0x4(%ebp),%eax
 651:	8b 10                	mov    (%eax),%edx
 653:	8b 45 f8             	mov    -0x8(%ebp),%eax
 656:	89 10                	mov    %edx,(%eax)
  if(p + p->s.size == bp){
 658:	8b 45 fc             	mov    -0x4(%ebp),%eax
 65b:	8b 40 04             	mov    0x4(%eax),%eax
 65e:	8d 14 c5 00 00 00 00 	lea    0x0(,%eax,8),%edx
 665:	8b 45 fc             	mov    -0x4(%ebp),%eax
 668:	01 d0                	add    %edx,%eax
 66a:	3b 45 f8             	cmp    -0x8(%ebp),%eax
 66d:	75 20                	jne    68f <free+0xcf>
    p->s.size += bp->s.size;
 66f:	8b 45 fc             	mov    -0x4(%ebp),%eax
 672:	8b 50 04             	mov    0x4(%eax),%edx
 675:	8b 45 f8             	mov    -0x8(%ebp),%eax
 678:	8b 40 04             	mov    0x4(%eax),%eax
 67b:	01 c2                	add    %eax,%edx
 67d:	8b 45 fc             	mov    -0x4(%ebp),%eax
 680:	89 50 04             	mov    %edx,0x4(%eax)
    p->s.ptr = bp->s.ptr;
 683:	8b 45 f8             	mov    -0x8(%ebp),%eax
 686:	8b 10                	mov    (%eax),%edx
 688:	8b 45 fc             	mov    -0x4(%ebp),%eax
 68b:	89 10                	mov    %edx,(%eax)
 68d:	eb 08                	jmp    697 <free+0xd7>
  } else
    p->s.ptr = bp;
 68f:	8b 45 fc             	mov    -0x4(%ebp),%eax
 692:	8b 55 f8             	mov    -0x8(%ebp),%edx
 695:	89 10                	mov    %edx,(%eax)
  freep = p;
 697:	8b 45 fc             	mov    -0x4(%ebp),%eax
 69a:	a3 40 0a 00 00       	mov    %eax,0xa40
}
 69f:	c9                   	leave  
 6a0:	c3                   	ret    

000006a1 <morecore>:

static Header*
morecore(uint nu)
{
 6a1:	55                   	push   %ebp
 6a2:	89 e5                	mov    %esp,%ebp
 6a4:	83 ec 28             	sub    $0x28,%esp
  char *p;
  Header *hp;

  if(nu < 4096)
 6a7:	81 7d 08 ff 0f 00 00 	cmpl   $0xfff,0x8(%ebp)
 6ae:	77 07                	ja     6b7 <morecore+0x16>
    nu = 4096;
 6b0:	c7 45 08 00 10 00 00 	movl   $0x1000,0x8(%ebp)
  p = sbrk(nu * sizeof(Header));
 6b7:	8b 45 08             	mov    0x8(%ebp),%eax
 6ba:	c1 e0 03             	shl    $0x3,%eax
 6bd:	89 04 24             	mov    %eax,(%esp)
 6c0:	e8 4f fc ff ff       	call   314 <sbrk>
 6c5:	89 45 f4             	mov    %eax,-0xc(%ebp)
  if(p == (char*)-1)
 6c8:	83 7d f4 ff          	cmpl   $0xffffffff,-0xc(%ebp)
 6cc:	75 07                	jne    6d5 <morecore+0x34>
    return 0;
 6ce:	b8 00 00 00 00       	mov    $0x0,%eax
 6d3:	eb 22                	jmp    6f7 <morecore+0x56>
  hp = (Header*)p;
 6d5:	8b 45 f4             	mov    -0xc(%ebp),%eax
 6d8:	89 45 f0             	mov    %eax,-0x10(%ebp)
  hp->s.size = nu;
 6db:	8b 45 f0             	mov    -0x10(%ebp),%eax
 6de:	8b 55 08             	mov    0x8(%ebp),%edx
 6e1:	89 50 04             	mov    %edx,0x4(%eax)
  free((void*)(hp + 1));
 6e4:	8b 45 f0             	mov    -0x10(%ebp),%eax
 6e7:	83 c0 08             	add    $0x8,%eax
 6ea:	89 04 24             	mov    %eax,(%esp)
 6ed:	e8 ce fe ff ff       	call   5c0 <free>
  return freep;
 6f2:	a1 40 0a 00 00       	mov    0xa40,%eax
}
 6f7:	c9                   	leave  
 6f8:	c3                   	ret    

000006f9 <malloc>:

void*
malloc(uint nbytes)
{
 6f9:	55                   	push   %ebp
 6fa:	89 e5                	mov    %esp,%ebp
 6fc:	83 ec 28             	sub    $0x28,%esp
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 6ff:	8b 45 08             	mov    0x8(%ebp),%eax
 702:	83 c0 07             	add    $0x7,%eax
 705:	c1 e8 03             	shr    $0x3,%eax
 708:	83 c0 01             	add    $0x1,%eax
 70b:	89 45 ec             	mov    %eax,-0x14(%ebp)
  if((prevp = freep) == 0){
 70e:	a1 40 0a 00 00       	mov    0xa40,%eax
 713:	89 45 f0             	mov    %eax,-0x10(%ebp)
 716:	83 7d f0 00          	cmpl   $0x0,-0x10(%ebp)
 71a:	75 23                	jne    73f <malloc+0x46>
    base.s.ptr = freep = prevp = &base;
 71c:	c7 45 f0 38 0a 00 00 	movl   $0xa38,-0x10(%ebp)
 723:	8b 45 f0             	mov    -0x10(%ebp),%eax
 726:	a3 40 0a 00 00       	mov    %eax,0xa40
 72b:	a1 40 0a 00 00       	mov    0xa40,%eax
 730:	a3 38 0a 00 00       	mov    %eax,0xa38
    base.s.size = 0;
 735:	c7 05 3c 0a 00 00 00 	movl   $0x0,0xa3c
 73c:	00 00 00 
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 73f:	8b 45 f0             	mov    -0x10(%ebp),%eax
 742:	8b 00                	mov    (%eax),%eax
 744:	89 45 f4             	mov    %eax,-0xc(%ebp)
    if(p->s.size >= nunits){
 747:	8b 45 f4             	mov    -0xc(%ebp),%eax
 74a:	8b 40 04             	mov    0x4(%eax),%eax
 74d:	3b 45 ec             	cmp    -0x14(%ebp),%eax
 750:	72 4d                	jb     79f <malloc+0xa6>
      if(p->s.size == nunits)
 752:	8b 45 f4             	mov    -0xc(%ebp),%eax
 755:	8b 40 04             	mov    0x4(%eax),%eax
 758:	3b 45 ec             	cmp    -0x14(%ebp),%eax
 75b:	75 0c                	jne    769 <malloc+0x70>
        prevp->s.ptr = p->s.ptr;
 75d:	8b 45 f4             	mov    -0xc(%ebp),%eax
 760:	8b 10                	mov    (%eax),%edx
 762:	8b 45 f0             	mov    -0x10(%ebp),%eax
 765:	89 10                	mov    %edx,(%eax)
 767:	eb 26                	jmp    78f <malloc+0x96>
      else {
        p->s.size -= nunits;
 769:	8b 45 f4             	mov    -0xc(%ebp),%eax
 76c:	8b 40 04             	mov    0x4(%eax),%eax
 76f:	2b 45 ec             	sub    -0x14(%ebp),%eax
 772:	89 c2                	mov    %eax,%edx
 774:	8b 45 f4             	mov    -0xc(%ebp),%eax
 777:	89 50 04             	mov    %edx,0x4(%eax)
        p += p->s.size;
 77a:	8b 45 f4             	mov    -0xc(%ebp),%eax
 77d:	8b 40 04             	mov    0x4(%eax),%eax
 780:	c1 e0 03             	shl    $0x3,%eax
 783:	01 45 f4             	add    %eax,-0xc(%ebp)
        p->s.size = nunits;
 786:	8b 45 f4             	mov    -0xc(%ebp),%eax
 789:	8b 55 ec             	mov    -0x14(%ebp),%edx
 78c:	89 50 04             	mov    %edx,0x4(%eax)
      }
      freep = prevp;
 78f:	8b 45 f0             	mov    -0x10(%ebp),%eax
 792:	a3 40 0a 00 00       	mov    %eax,0xa40
      return (void*)(p + 1);
 797:	8b 45 f4             	mov    -0xc(%ebp),%eax
 79a:	83 c0 08             	add    $0x8,%eax
 79d:	eb 38                	jmp    7d7 <malloc+0xde>
    }
    if(p == freep)
 79f:	a1 40 0a 00 00       	mov    0xa40,%eax
 7a4:	39 45 f4             	cmp    %eax,-0xc(%ebp)
 7a7:	75 1b                	jne    7c4 <malloc+0xcb>
      if((p = morecore(nunits)) == 0)
 7a9:	8b 45 ec             	mov    -0x14(%ebp),%eax
 7ac:	89 04 24             	mov    %eax,(%esp)
 7af:	e8 ed fe ff ff       	call   6a1 <morecore>
 7b4:	89 45 f4             	mov    %eax,-0xc(%ebp)
 7b7:	83 7d f4 00          	cmpl   $0x0,-0xc(%ebp)
 7bb:	75 07                	jne    7c4 <malloc+0xcb>
        return 0;
 7bd:	b8 00 00 00 00       	mov    $0x0,%eax
 7c2:	eb 13                	jmp    7d7 <malloc+0xde>
  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 7c4:	8b 45 f4             	mov    -0xc(%ebp),%eax
 7c7:	89 45 f0             	mov    %eax,-0x10(%ebp)
 7ca:	8b 45 f4             	mov    -0xc(%ebp),%eax
 7cd:	8b 00                	mov    (%eax),%eax
 7cf:	89 45 f4             	mov    %eax,-0xc(%ebp)
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
 7d2:	e9 70 ff ff ff       	jmp    747 <malloc+0x4e>
}
 7d7:	c9                   	leave  
 7d8:	c3                   	ret    
