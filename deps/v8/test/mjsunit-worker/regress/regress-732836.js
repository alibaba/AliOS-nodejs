// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-732836.js");
	}
}
function boom() {
  var args = [];
  for (var i = 0; i < 125000; i++)
    args.push(1.1);
  return Array.apply(Array, args);
}
var array = boom();
