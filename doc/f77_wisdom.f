c     This is an example implementation of Fortran wisdom export/import
c     to/from a Fortran unit (file), exploiting the generic
c     dfftw_export_wisdom/dfftw_import_wisdom functions.

c     Strictly speaking, the '$' format specifier, which allows us to
c     write a character without a trailing newline, is not standard F77.
c     However, it seems to be a nearly universal extension.
      subroutine absorb(c, iunit)
      character c
      integer iunit
      write(iunit,321) c
 321  format(a,$)
      end      

      subroutine export_wisdom_to_file(iunit)
      integer iunit
      external absorb
      call dfftw_export_wisdom(absorb, iunit)
      end

c     Fortran 77 does not have any portable way to read an arbitrary
c     file one character at a time.  The best alternative seems to be to
c     read a whole line into a buffer, since for fftw-exported wisdom we
c     can bound the line length.  (If the file contains longer lines,
c     then the lines will truncated and the wisdom import should simply
c     fail.)  Ugh.
      subroutine emit(ic, iunit)
      integer ic
      integer iunit
      character*256 buf
      save buf
      integer ibuf
      data ibuf/257/
      save ibuf
      if (ibuf .lt. 257) then
         ic = ichar(buf(ibuf:ibuf))
         ibuf = ibuf + 1
         return
      endif
      read(iunit,123,end=666) buf
      ic = ichar(buf(1:1))
      ibuf = 2
      return
 666  ic = -1
      ibuf = 257
 123  format(a256)
      end
      
      subroutine import_wisdom_from_file(isuccess, iunit)
      integer isuccess
      integer iunit
      external emit
      call dfftw_import_wisdom(isuccess, emit, iunit)
      end
