// 
//  UriExtensions.cs
// 
//  Author:
//   Jim Borden  <jim.borden@couchbase.com>
// 
//  Copyright (c) 2017 Couchbase, Inc All rights reserved.
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 

using System;
using System.Collections.Specialized;

using JetBrains.Annotations;

namespace Couchbase.Lite.Testing
{
    internal static class UriExtensions
    {
        public static NameValueCollection ParseQueryString([NotNull]this Uri url)
        {
            var retVal = new NameValueCollection();
            if (String.IsNullOrEmpty(url.Query)) {
                return retVal;
            }

            foreach (var pair in url.Query.Substring(1).Split('&')) {
                var nameValue = pair.Split('=');
                retVal.Add(nameValue[0], nameValue[1]);
            }

            return retVal;
        }
    }
}