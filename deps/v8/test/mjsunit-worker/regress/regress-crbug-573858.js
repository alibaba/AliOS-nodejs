// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-573858.js");
	}
}
var throw_type_error = Object.getOwnPropertyDescriptor(
    (function() {"use strict"}).__proto__, "caller").get;

function create_initial_map() { this instanceof throw_type_error }
%OptimizeFunctionOnNextCall(create_initial_map);
assertThrows(create_initial_map);

function test() { new throw_type_error }
%OptimizeFunctionOnNextCall(test);
assertThrows(test);
