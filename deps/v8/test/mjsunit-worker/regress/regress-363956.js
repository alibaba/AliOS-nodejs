// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-363956.js");
	}
}
function Fuu() { this.x = this.x.x; }
Fuu.prototype.x = {x: 1}
new Fuu();
new Fuu();
%OptimizeFunctionOnNextCall(Fuu);
new Fuu();
