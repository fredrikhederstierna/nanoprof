#
# Make script for building trace gui application.
#
# Fredrik Hederstierna 2014

JC = javac
JR = java
JAR = jar

BUILD = ../build_nanoprof

all:
	$(JC) -d $(BUILD) -Xlint:unchecked -cp .:$(CLASSPATH) host/*.java
run:
	$(JR) -classpath $(BUILD) Main
elf:
	$(JR) -classpath $(BUILD) Addr2Line
jar:
	$(JAR) cvfm gui.jar manifest.txt -C $(BUILD) "" $(BUILD)/*.class
clean:
	rm $(BUILD)/*.class
