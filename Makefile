archive:
	xcodebuild -project casper.xcodeproj -scheme casper clean archive -configuration release -archivePath /tmp/casper.xcarchive

cef:
	@echo "ACTION=$(ACTION)"
	$(shell zsh ./tpf.zsh $(ACTION))
