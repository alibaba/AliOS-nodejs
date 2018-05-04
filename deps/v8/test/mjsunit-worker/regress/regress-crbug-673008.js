// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-673008.js");
	}
}
var a = {
  "33": true,
  "-1": true
};

var strkeys = Object.keys(a).map(function(k) { return "" + k });
var numkeys = Object.keys(a).map(function(k) { return +k });
var keys = strkeys.concat(numkeys);

keys.forEach(function(k) {
  assertTrue(a.hasOwnProperty(k),
             "property not found: " + k + "(" + (typeof k) + ")");
});

var b = {};
b.__proto__ = a;
keys.forEach(function(k) {
  assertTrue(k in b, "property not found: " + k + "(" + (typeof k) + ")");
});
