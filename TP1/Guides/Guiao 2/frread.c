int frread(int fd, unsigned char * buf, int maxlen){
	int n=0;
	int ch;
	while(1){
		if((ch=read(fd, buf+n, 1)) <=0){
			return ch; // error...
		}
		if(n==0 && buf[n]!=FRFLAG){
			continue;
		}
		if(n==1 && buf[n]==FRFLAG){
			continue;
		}	
		n++;
		if(buf[n-1]!=FRFLAG && n==maxlen){
			n=0;
			continue;
		}
		if(buf[n-1]==FFLAG && n>2){
			//processrframe(buf,n);
		return n;
		{
	}	
}
