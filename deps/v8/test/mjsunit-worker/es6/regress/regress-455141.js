"use strict"
// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Flags: --no-lazy
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-455141.js");
	}
}
"use strict";
class Base {
}
class Subclass extends Base {
  constructor() {
      this.prp1 = 3;
  }
}
function __f_1(){
}
