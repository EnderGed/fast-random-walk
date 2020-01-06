#/bin/bash --

if [$? == 1]; then echo "ERROR"; fi
for exec in $(EXEC); do\
	echo "Testing ./$$exec";\
	./$$exec;\
	echo "Done $$exec";\
done
