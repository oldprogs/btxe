/* stop all mailer lines */
do i=1 to 5
  call lineout 'Flags\BTEXIT01.'d2x(i,2)
end
