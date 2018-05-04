// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --gc-interval=33 --expose-gc --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-474297.js");
	}
}
var Debug = %GetDebugContext().Debug;
Debug.setListener(function(){});

%DebugGetLoadedScripts();

Debug.setListener(null);
