// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --gc-interval=30

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-630561.js");
	}
}
var dict_elements = {};

for (var i= 0; i< 100; i++) {
  dict_elements[2147483648 + i] = i;
}

var keys = Object.keys(dict_elements);
