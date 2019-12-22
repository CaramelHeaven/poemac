//
//  configData.swift
//  appFirewall
//
//  Created by Doug Leith on 04/12/2019.
//  Copyright © 2019 Doug Leith. All rights reserved.
//

import Foundation

class Config: NSObject {
	// fixed settings ...
	static let defaultLoggingLevel = 2 // more verbose, for testing
	static let enableDtrace = 1 // disable for SIP testing
	static let enableUpdates = 0 // disable for testing
	
	static let minHelperVersion = 5 // required helper version, must be an Int
	
	static let crashURL = URL(string: "https://leith.ie/logcrash.php")!
	static let sampleURL = URL(string: "https://leith.ie/logsample.php")!
	static let updateCheckURL = URL(string: "https://github.com/doug-leith/appFirewall/raw/master/version")!
	static let updateURL = "https://github.com/doug-leith/appFirewall/raw/master/latest%20release/appFirewall.dmg"
	static let checkUpdatesInterval = 2592000.0 // 30 days, in seconds !
	
	static let browsers = ["firefox","Google Chrome H","Safari","Opera Helper","Brave Browser H","seamonkey"]
	
	static let csrutil = "/usr/bin/csrutil"
	static let pgrep = "/usr/bin/pgrep"
	
	static let defaultNameList = ["host_lists":["Energized Blu (Recommended)"],]
	static let hostNameLists : [[String: String]] =
	[
		["Name":"Energized Blu (Recommended)", "File": "energized_blu.txt", "URL": "https://block.energized.pro/blu/formats/hosts","Tip":"A large, quite complete list (231K entries).", "Type":"Hostlist"],
		["Name":"Steve Black Unified", "File": "steve_black_unified.txt", "URL": "https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts","Tip":"A good choice that tries to keep balance between blocking effectiveness and false positives.  Includes Dan Pollock's, MVPS, AdAway lists amongst other.  However, doesn't cover Irish specific trackers such as adservice.google.ie", "Type":"Hostlist"],
		["Name": "Goodbye Ads by Jerryn70","File":"GoodbyeAds.txt",  "URL":"https://raw.githubusercontent.com/jerryn70/GoodbyeAds/master/Hosts/GoodbyeAds.txt","Tip":"Blocks mobile ads and trackers, including blocks ads by Facebook.  Includes Irish specific trackers such as adservice.google.ie", "Type":"Hostlist"],
		["Name": "Dan Pollock's hosts file","File": "hosts","URL":"https://someonewhocares.org/hosts/zero/hosts","Tip":"A balanced ad blocking hosts file.  Try this if other ones are blocking too much.", "Type":"Hostlist"],
		["Name": "AdAway","File":"hosts.txt","URL":"https://adaway.org/hosts.txt","Tip":"Blocks ads and some analytics but quite limited (only 525 hosts)", "Type":"Hostlist"],
		["Name": "hpHosts","File": "ad_servers.txt" ,"URL":"https://hosts-file.net/ad_servers.txt", "Tip":"Ad and trackers list from hpHosts, moderately sizesd (45K hosts)."],
		["Name": "Doug's Annoyances Block List","File": "dougs_blocklist.txt","URL": "https://www.scss.tcd.ie/doug.leith/dougs_blocklist.txt", "Tip": "Based on MAC OS application traffic.", "Type":"Blocklist"]
		//["Name": "","File": "","URL": "", "Tip": "", "Type":"Hostlist"]
	]
	
	static let logName = "log.dat"
	static let logTxtName = "log.txt" // human readable log file
	static let appLogName = "app_log.txt"
	static let dnsName = "dns.dat"
	static let blockListName = "blocklist.dat"
	static let whiteListName = "whitelist.dat"
	static let dnsConnListName = "dns_connlist.dat"
	
	static let appDelegateRefreshTime : Double = 10 // check state every 10s
	static let viewRefreshTime : Double = 1 // check for window uodate every 1s

	//------------------------------------------------
	// settings that can be changed by user ...
	static var checkUpdateTimer : Timer = Timer() // timer for updates

	@objc static func doTimedCheckForUpdate() {
		// used for timed update checking
		print("doTimedCheckForUpdate")
		UserDefaults.standard.register(defaults: ["autoUpdate":true])
		let autoUpdate = UserDefaults.standard.bool(forKey: "autoUpdate")
		doCheckForUpdates(quiet: true, autoUpdate: autoUpdate)
	}
	
	static func initTimedCheckForUpdate() {
		UserDefaults.standard.register(defaults: ["autoCheckUpdates":true])
		print("initTimedCheckForUpdate: autoCheckUpdates ",UserDefaults.standard.bool(forKey: "autoCheckUpdates"))
		if UserDefaults.standard.bool(forKey: "autoCheckUpdates") {
			checkUpdateTimer = Timer.scheduledTimer(timeInterval: Config.checkUpdatesInterval, target: self, selector: #selector(doTimedCheckForUpdate), userInfo: nil, repeats: true)
		} else {
			checkUpdateTimer.invalidate()
		}
	}

	static func refresh() {
		initTimedCheckForUpdate()
		// runAtLogin update
	}
	
	static func autoCheckUpdates(value: Bool) {
		UserDefaults.standard.set(value, forKey: "autoCheckUpdates")
	}
	
	static func autoUpdate(value: Bool) {
		UserDefaults.standard.set(value, forKey: "autoUpdate")
	}
	
	static func runAtLogin(value: Bool) {
		UserDefaults.standard.set(value, forKey: "runAtLogin")
	}

	static func getSetting(label: String, def: Bool)->Bool {
		UserDefaults.standard.register(defaults: [label: def])
		return UserDefaults.standard.bool(forKey: label)
	}
	
	static func getAutoCheckUpdates()->Bool {
		return getSetting(label: "autoCheckUpdates", def: true)
	}

	static func getAutoUpdate()->Bool {
		return getSetting(label: "autoUpdate", def: true)
	}

	static func getRunAtLogin()->Bool {
		return getSetting(label: "runAtLogin", def: true)
	}

}