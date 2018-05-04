// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/generic-add.js");
	}
}
function dateL() {
  var date = new Date();
  return (date + true) == date.toString() + true;
}

function dateR() {
  var date = new Date();
  return (true + date) == true + date.toString();
}

function strL() {
  return (new String(1) + true) == "1true";
}

function strR() {
  return (true + new String(1)) == "true1";
}

assertTrue(dateL());
assertTrue(dateR());
assertTrue(strL());
assertTrue(strR());
