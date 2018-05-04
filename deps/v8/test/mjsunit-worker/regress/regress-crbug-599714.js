// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-599714.js");
	}
}
var custom_toString = function() {
  var boom = custom_toString.caller;
  return boom;
}

var object = {};
object.toString = custom_toString;

try { Object.hasOwnProperty(object); } catch (e) {}
