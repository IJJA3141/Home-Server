link:
	find ./out/Debug/ -type l -delete
	ln -t out/Debug -rs server/*
