// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-448711.js");
	}
}
function f() {
  this.a = { text: "Hello!" };
}
var v4 = new f();
var v7 = new f();
v7.b = {};
Object.defineProperty(v4, '2', {});
var v6 = new f();
v6.a = {};
