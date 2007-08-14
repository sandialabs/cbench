      program recvbug2

      include 'mpif.h'

      integer, parameter :: MSG_SIZE = 9
      integer, parameter :: COOKIE = 8301965
      
      integer, allocatable :: the_matrix(:,:)
      integer, allocatable :: send_buf(:)
      integer, allocatable :: recv_buf(:)
      integer, allocatable :: request(:)
      integer, allocatable :: status(:)
      
      integer i, j, k, m
      integer rank, nproc
      integer nsend, nrecv, nneigh
      integer irecv, isend, size, ioffset
      integer ierror
      integer iter
      integer random_count
      
      
      call MPI_Init(ierror)
      
      call MPI_Get_version(major,minor,ierror)

      call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierror)
      call MPI_Comm_size(MPI_COMM_WORLD, nproc, ierror)
      
      if (rank .eq. 0) print *,"MPI Major = ",major,".",minor

      allocate(the_matrix(0:nproc-1,0:nproc-1))
      allocate(request(nproc))
      allocate(status(MPI_STATUS_SIZE*nproc))
      
      iter = 0
      do while (.true.)
         
! ...... the_matrix(from,to) = MSG_SIZE words to send
         the_matrix = 0
         do i=0,nproc-1
            do j=0,nproc-1
               if (i .ne. j) then
                  the_matrix(i,j) = random_count()
               end if
            end do
         end do
         
         nneigh = nproc - 1
         nsend = 0
         nrecv = 0
         do i=0,nproc-1
            nsend = nsend + the_matrix(rank,i)
            nrecv = nrecv + the_matrix(i,rank)
         end do
         allocate(send_buf(nsend*MSG_SIZE))
         allocate(recv_buf(nrecv*MSG_SIZE))
         
         do i=1,nrecv*MSG_SIZE
            recv_buf(i) = -i
         end do
         
         irecv = 0
         ioffset = 1
         do i=0,nneigh
            size = MSG_SIZE*the_matrix(i,rank)
            if (size .gt. 0) then
               irecv = irecv + 1
               call MPI_Irecv(recv_buf(ioffset), size, MPI_INTEGER,     &
               &              i, 0, MPI_COMM_WORLD, request(irecv), ierror)
               ioffset = ioffset + size
            end if
         end do
         
         m = 0
         do i=0,nneigh
            ioffset = m + 1
            size = 0
            
            do j=1,the_matrix(rank,i)
               do k=1,MSG_SIZE
                  m = m + 1
                  size = size + 1
                  send_buf(m) = COOKIE + k
               end do
            end do
            if (size .gt. 0) then
               call MPI_Send(send_buf(ioffset),size,MPI_INTEGER,        &
               &                       i, 0, MPI_COMM_WORLD, ierror)
            end if
         end do
         
         if (irecv .gt. 0) then
            call MPI_Waitall(irecv,request,status,ierror)
            if (ierror .ne. 0) then
               stop 'recvbug2: error on MPI_Waitall'
            end if
         end if
         
         m = 0
         do i=1,nneigh
            do j=1,the_matrix(i,rank)
               do k=1,MSG_SIZE
                  m = m + 1
                  if (recv_buf(m) .ne. COOKIE+k) then
                     write (6,*) 'recvbug2: error on recv!',            &
                     &                    ' proc=', rank,                               &
                     &                    ' particle=', j, ' of ', the_matrix(i,rank),  &
                     &                    ' word=', k
                  end if
               end do
            end do
         end do
         
         call MPI_Barrier(MPI_COMM_WORLD, ierror)
         
         deallocate(send_buf)
         deallocate(recv_buf)
      if ( (mod(iter,1000) .eq. 0 ) .and. (rank .eq. 0)) print *, "iteration = ",iter
      iter = iter + 1         
      end do
      
      deallocate(the_matrix)
      deallocate(request)
      deallocate(status)
      
      call MPI_Finalize(ierror)
      
      end
      
      
      integer function random_count()
      real r
      call random_number(r)
      random_count = int(100*r)
      if (random_count .gt. 75) random_count = 0
      return
      end
      
