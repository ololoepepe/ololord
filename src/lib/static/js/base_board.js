/*ololord global object*/

var lord = lord || {};

/*Variables*/

lord.postPreviews = {};
lord.lastPostPreview = null;
lord.lastPostPreviewTimer = null;
lord.images = {};
lord.img = null;
lord.postForm = {
    "visibility": {
        "Top": false,
        "Bottom": false
    },
    "last": "",
    "quickReply": false
};
lord.complainVideo = null;
lord.files = null;
lord.filesMap = null;

/*Functions*/

lord.isAudioType = function(type) {
    return type in {"audio/mpeg": true, "audio/ogg": true, "audio/wav": true};
};

lord.isImageType = function(type) {
    return type in {"image/gif": true, "image/jpeg": true, "image/png": true};
};

lord.isVideoType = function(type) {
    return type in {"video/mp4": true, "video/ogg": true, "video/webm": true};
};

lord.isSpecialThumbName = function(thumbName) {
    return lord.isAudioType(thumbName) || lord.isImageType(thumbName) || lord.isVideoType(thumbName);
};

lord.resetScale = function(image) {
    var k = (image.scale / 100);
    var tr = "scale(" + k + ", " + k + ")";
    //Fuck you all who create those stupid browser-specific features
    image.style.webkitTransform = tr;
    image.style.MozTransform = tr;
    image.style.msTransform = tr;
    image.style.OTransform = tr;
    image.style.transform = tr;
};

lord.removeReferences = function(postNumber) {
    postNumber = +postNumber;
    if (isNaN(postNumber))
        return;
    var referencedByTrs = lord.name("referencedByTr");
    if (!referencedByTrs)
        return;
    for (var i = 0; i < referencedByTrs.length; ++i) {
        var referencedByTr = referencedByTrs[i];
        var as = lord.query("a", referencedByTr);
        if (!as)
            continue;
        for (var j = 0; j < as.length; ++j) {
            var a = as[j];
            if (a.innerHTML == ("&gt;&gt;" + postNumber)) {
                a.parentNode.removeChild(a);
                if (as.length < 2)
                    referencedByTr.style.display = "none";
                break;
            }
        }
    }
};

lord.addReferences = function(postNumber, referencedPosts) {
    postNumber = +postNumber;
    if (isNaN(postNumber))
        return;
    if (!referencedPosts)
        return;
    var prefix = lord.text("sitePathPrefix");
    var currentBoardName = lord.text("currentBoardName");
    for (key in referencedPosts) {
        if (!referencedPosts.hasOwnProperty(key))
            continue;
        var bn = key.split("/").shift();
        if (bn !== currentBoardName)
            continue;
        var pn = key.split("/").pop();
        var tn = referencedPosts[key];
        var post = lord.id("post" + pn);
        if (!post)
            continue;
        var referencedByTr = lord.nameOne("referencedByTr", post);
        referencedByTr.style.display = "";
        var referencedBy = lord.nameOne("referencedBy", post);
        var a = lord.node("a");
        a.href = "/" + prefix + bn + "/thread/" + tn + ".html#" + postNumber;
        referencedBy.appendChild(lord.node("text", " "));
        a.appendChild(lord.node("text", ">>" + postNumber));
        referencedBy.appendChild(a);
    }
};

lord.setInitialScale = function(image, sizeHintX, sizeHintY) {
    if (!sizeHintX || !sizeHintY || sizeHintX <= 0 || sizeHintY <= 0)
        return;
    var doc = document.documentElement;
    var maxWidth = doc.clientWidth - 10;
    var maxHeight = doc.clientHeight - 10;
    var kw = 1;
    var kh = 1;
    if (sizeHintX > maxWidth)
        kw = maxWidth / sizeHintX;
    if (sizeHintY > maxHeight)
        kh = maxHeight / sizeHintY;
    image.scale = ((kw < kh) ? kw : kh) * 100;
};

lord.reloadCaptchaFunction = function() {
    if (!!grecaptcha)
        grecaptcha.reset();
};

lord.resetCaptcha = function() {
    var captcha = lord.id("captcha");
    if (!!captcha) {
        var boardName = lord.text("currentBoardName");
        lord.ajaxRequest("get_captcha_quota", [boardName], lord.RpcGetCaptchaQuotaId, function(res) {
            res = +res;
            if (isNaN(res))
                return;
            var hiddenCaptcha = lord.id("hiddenCaptcha");
            var td = lord.id("captchaContainer");
            for (var i = 0; i < td.children.length; ++i) {
                if (td.children[i] == captcha)
                    continue;
                td.removeChild(td.children[i]);
            }
            if (res > 0) {
                hiddenCaptcha.appendChild(captcha);
                var span = lord.node("span");
                lord.addClass(span, "noCaptchaText");
                var text = lord.text("noCaptchaText") + ". " + lord.text("captchaQuotaText") + " " + res;
                span.appendChild(lord.node("text", text));
                td.appendChild(span);
            } else {
                lord.id("captchaContainer").appendChild(captcha);
            }
            if (!!lord.reloadCaptchaFunction && "hiddenCaptcha" !== captcha.parentNode.id)
                lord.reloadCaptchaFunction();
        });
    }
};

lord.traverseChildren = function(elem) {
    var children = [];
    var q = [];
    q.push(elem);
    function pushAll(elemArray) {
        for (var i = 0; i < elemArray.length; ++i)
            q.push(elemArray[i]);
    }
    while (q.length > 0) {
        var elem = q.pop();
        children.push(elem);
        pushAll(elem.children);
    }
    return children;
};

lord.createPostFile = function(f, boardName, postNumber) {
    if (!f || !boardName || isNaN(+postNumber))
        return null;
    var sitePrefix = lord.text("sitePathPrefix");
    if (!boardName)
        boardName = lord.text("currentBoardName");
    var file = lord.node("td");
    file.id = "file" + f["sourceName"];
    lord.addClass(file, "postFile");
    if (lord.isAudioType(f["type"])) {
        ["Album", "Artist", "Title", "Year"].forEach(function(key) {
            var inp = lord.node("input");
            inp.type = "hidden";
            inp.name = "audioTag" + key;
            inp.value = f["audioTag" + key];
            file.appendChild(inp);
        });
    }
    var divFileName = lord.node("div");
    lord.addClass(divFileName, "postFileName");
    var aFileName = lord.node("a");
    aFileName.href = "/" + sitePrefix + boardName + "/" + f["sourceName"];
    aFileName.target = "_blank";
    aFileName.appendChild(lord.node("text", f["sourceName"]));
    divFileName.appendChild(aFileName);
    file.appendChild(divFileName);
    var divFileSize = lord.node("div");
    lord.addClass(divFileSize, "postFileSize");
    divFileSize.appendChild(lord.node("text", "(" + f["size"] + ")"));
    divFileSize.title = f["sizeTooltip"];
    file.appendChild(divFileSize);
    var divFileSearch = lord.node("div");
    lord.addClass(divFileSearch, "postFileSearch");
    var a = lord.node("a");
    a.href = "javascript:lord.deleteFile('" + boardName + "', " + postNumber + ", '" + f["sourceName"] + "');";
    a.title = lord.text("deleteFileText");
    var logo = lord.node("img");
    logo.src = "/" + sitePrefix + "img/delete.png";
    a.appendChild(logo);
    divFileSearch.appendChild(a);
    if (lord.isImageType(f["type"])) {
        var siteDomain = lord.text("siteDomain");
        var siteProtocol = lord.text("siteProtocol");
        [{
            "link": "//www.google.com/searchbyimage?image_url=",
            "text": lord.text("findSourceWithGoogleText"),
            "img": "google.png"
        }, {
            "link": "http://iqdb.org/?url=",
            "text": lord.text("findSourceWithIqdbText"),
            "img": "iqdb.png"
        }].forEach(function(el) {
            var a = lord.node("a");
            a.href = el.link + siteProtocol + "://" + siteDomain + "/" + sitePrefix + boardName + "/" + f["sourceName"];
            a.title = el.text;
            a.target = "_blank";
            var logo = lord.node("img");
            logo.src = "/" + sitePrefix + "img/" + el.img;
            a.appendChild(logo);
            divFileSearch.appendChild(lord.node("text", " "));
            divFileSearch.appendChild(a);
        });
    }
    if (lord.isAudioType(f["type"])) {
        var a = lord.node("a");
        a.href = "javascript:lord.editAudioTags('" + boardName + "', " + postNumber + ", '" + f["sourceName"] + "');";
        a.title = lord.text("editAudioTagsText");
        var logo = lord.node("img");
        logo.src = "/" + sitePrefix + "img/audio_edit_tags.png";
        a.appendChild(logo);
        divFileSearch.appendChild(lord.node("text", " "));
        divFileSearch.appendChild(a);
        a = lord.node("a");
        a.href = "javascript:lord.addToPlaylist('" + boardName + "', '" + f["sourceName"] + "');";
        a.title = lord.text("addToPlaylistText");
        logo = lord.node("img");
        logo.src = "/" + sitePrefix + "img/playlist_add.png";
        a.appendChild(logo);
        divFileSearch.appendChild(lord.node("text", " "));
        divFileSearch.appendChild(a);
    }
    file.appendChild(divFileSearch);
    var divImage = lord.node("div");
    lord.addClass(divImage, "postFileFile");
    var inpType = lord.node("input");
    inpType.type = "hidden";
    inpType.name = "type";
    inpType.value = f["type"];
    divImage.appendChild(inpType);
    var inpSizeX = lord.node("input");
    inpSizeX.type = "hidden";
    inpSizeX.name = "sizeX";
    inpSizeX.value = f["sizeX"];
    divImage.appendChild(inpSizeX);
    var inpSizeY = lord.node("input");
    inpSizeY.type = "hidden";
    inpSizeY.name = "sizeY";
    inpSizeY.value = f["sizeY"];
    divImage.appendChild(inpSizeY);
    var aImage = lord.node("a");
    aImage.href = "/" + sitePrefix + boardName + "/" + f["sourceName"];
    aImage.onclick = lord.showImage.bind(lord, "/" + sitePrefix + boardName + "/" + f["sourceName"], f["type"],
        f["sizeX"], f["sizeY"]);
    var image = lord.node("img");
    var thumbSizeX = +f["thumbSizeX"];
    var thumbSizeY = +f["thumbSizeY"];
    if (!isNaN(thumbSizeX) && thumbSizeX > 0)
        image.width = thumbSizeX;
    if (!isNaN(thumbSizeY) && thumbSizeY > 0)
        image.height = thumbSizeY;
    if (lord.isSpecialThumbName(f["thumbName"])) {
        image.src = "/" + sitePrefix + "img/" + f["thumbName"].replace("/", "_") + "_logo.png";
    } else {
        image.src = "/" + sitePrefix + boardName + "/" + f["thumbName"];
    }
    aImage.appendChild(image);
    divImage.appendChild(aImage);
    file.appendChild(divImage);
    return file;
};

lord.createPostNode = function(res, permanent, boardName) {
    if (!res)
        return null;
    post = lord.id("postTemplate");
    if (!post)
        return null;
    post = post.cloneNode(true);
    post.id = !!permanent ? ("post" + res["number"]) : "";
    post.style.display = "";
    if (!boardName)
        boardName = lord.text("currentBoardName");
    var fixed = lord.nameOne("fixed", post);
    if (!!res["fixed"])
        fixed.style.display = "";
    else
        fixed.parentNode.removeChild(fixed);
    var closed = lord.nameOne("closed", post);
    if (!!res["closed"])
        closed.style.display = "";
    else
        closed.parentNode.removeChild(closed);
    var bumpLimit = lord.nameOne("bumpLimit", post);
    if (!!res["bumpLimitReached"])
        bumpLimit.style.display = "";
    else
        bumpLimit.parentNode.removeChild(bumpLimit);
    var postLimit = lord.nameOne("postLimit", post);
    if (!!res["postLimitReached"])
        postLimit.style.display = "";
    else
        postLimit.parentNode.removeChild(postLimit);
    lord.nameOne("postSubject", post).appendChild(lord.node("text", res["subject"]));
    var registered = lord.nameOne("registered", post);
    if (!!res["showRegistered"] && !!res["showTripcode"])
        registered.style.display = "";
    else
        registered.parentNode.removeChild(registered);
    var name = lord.nameOne("someName", post);
    if (!!lord.getLocalObject("hideUserNames", false))
        name.style.display = "none";
    if (!!res["email"])
        name.innerHTML = "<a class='mailtoName' href='mailto:" + res["email"] + "'>" + res["nameRaw"] + "</a>";
    else
        name.innerHTML = res["name"];
    var tripcode = lord.nameOne("tripcode", post);
    if (!!res["showTripcode"] && "" !== res["tripcode"]) {
        if (!lord.getLocalObject("hideTripcodes", false))
            tripcode.style.display = "";
        tripcode.appendChild(lord.node("text", res["tripcode"]));
    } else {
        tripcode.parentNode.removeChild(tripcode);
    }
    var whois = lord.nameOne("whois", post);
    var sitePathPrefix = lord.text("sitePathPrefix");
    if (!!res["flagName"]) {
        whois.style.display = "";
        whois.src = whois.src.replace("%flagName%", res["flagName"]);
        whois.title = res["countryName"];
        if (!!res["cityName"])
            whois.title += ": " + res["cityName"];
    } else {
        whois.parentNode.removeChild(whois);
    }
    lord.nameOne("dateTime", post).appendChild(lord.node("text", res["dateTime"]));
    var moder = (lord.text("moder") === "true");
    var number = lord.nameOne("number", post);
    number.appendChild(lord.node("text", res["number"]));
    if (moder)
        number.title = res["ip"];
    var postingEnabled = (lord.text("postingEnabled") === "true");
    var inp = lord.id("currentThreadNumber");
    if (!!inp && +inp.value === res["threadNumber"]) {
        number.href = "#" + res["number"];
    } else {
        number.href = "/" + sitePathPrefix + boardName + "/thread/" + res["threadNumber"] + ".html#"
        if (postingEnabled)
            number.href += "i";
        number.href += res["number"];
    }
    var files = lord.nameOne("files", post);
    if (!!res["files"]) {
        post.ajaxFiles = {};
        for (var i = 0; i < res["files"].length; ++i) {
            var file = lord.createPostFile(res["files"][i], boardName, res["number"]);
            if (!!file)
                files.insertBefore(file, files.children[files.children.length - 1]);
            post.ajaxFiles[res["files"][i]["thumbName"]] = res["files"][i];
        }
    }
    var blockquoteThread = !!lord.id("currentThreadNumber");
    var modificationDateTimeTd = lord.nameOne("modificationDateTimeTd", post);
    var bannedForTd = lord.nameOne("bannedForTd", post);
    var referencedByTd = lord.nameOne("referencedByTd", post);
    var oneFileTr = lord.nameOne("files", post);
    var manyFilesTr = lord.nameOne("manyFilesTr", post);
    var textOneFile = lord.nameOne("textOneFile", post);
    var textManyFiles = lord.nameOne("textManyFiles", post);
    if (res["files"].length < 2) {
        manyFilesTr.parentNode.removeChild(manyFilesTr);
        textOneFile.innerHTML = res["text"];
        if (blockquoteThread)
            lord.addClass(textOneFile, "blockquoteThread");
    } else {
        var oneFileTd = lord.nameOne("oneFileTd", post);
        oneFileTd.removeChild(textOneFile);
        lord.addClass(oneFileTd, "shrink");
        lord.nameOne("manyFilesTd", post).colSpan = res["files"].length + 2;
        lord.nameOne("textManyFiles", post).innerHTML = res["text"];
        if (blockquoteThread)
            lord.addClass(textManyFiles, "blockquoteThread");
        modificationDateTimeTd.colSpan = res["files"].length + 2;
        bannedForTd.colSpan = res["files"].length + 2;
        referencedByTd.colSpan = res["files"].length + 2;
    }
    var modificationDateTime = lord.nameOne("modificationDateTime", post);
    if ("" !== res["modificationDateTime"]) {
        modificationDateTime.style.display = "";
        modificationDateTime.childNodes[0].nodeValue += " " + res["modificationDateTime"];
    } else {
        modificationDateTime.parentNode.removeChild(modificationDateTime);
    }
    var bannedFor = lord.nameOne("bannedFor", post);
    if (!!res["bannedFor"])
        bannedFor.style.display = "";
    else
        bannedFor.parentNode.removeChild(bannedFor);
    var referencedBy = lord.nameOne("referencedBy", post);
    if (!!res["referencedBy"] && res["referencedBy"].length > 0) {
        lord.nameOne("referencedByTr", post).style.display = "";
        for (var i = 0; i < res["referencedBy"].length; ++i) {
            var ref = res["referencedBy"][i];
            var bn = ref["boardName"]
            var pn = ref["postNumber"];
            var tn = ref["threadNumber"];
            var a = lord.node("a");
            a.href = "/" + sitePathPrefix + bn + "/thread/" + tn + ".html#" + pn;
            referencedBy.appendChild(lord.node("text", " "));
            a.appendChild(lord.node("text", ">>" + (bn !== boardName ? ("/" + bn + "/") : "") + pn));
            referencedBy.appendChild(a);
        }
    }
    if (lord.createPostNodeCustom)
        lord.createPostNodeCustom(post, res, permanent, boardName);
    var perm = lord.nameOne("permanent", post);
    if (!permanent) {
        perm.parentNode.removeChild(perm);
        lord.addClass(post, "temporary");
        return post;
    }
    var quickReply = lord.nameOne("quickReply", post);
    quickReply.href = quickReply.href.replace("%postNumber%", res["number"]);
    lord.removeReferences(res["number"]);
    if (!!res["refersTo"]) {
        var refersTo = {};
        for (var i = 0; i < res["refersTo"].length; ++i) {
            var v = res["refersTo"][i];
            refersTo[v["boardName"] + "/" + v["postNumber"]] = v["threadNumber"];
        }
        lord.addReferences(res["number"], refersTo);
    }
    lord.addClass(post, "newPost");
    post.onmouseover = function() {
        lord.removeClass(this, "newPost");
        this.onmouseover = null;
    }
    if (res["number"] === res["threadNumber"]) {
        lord.removeClass(post, "post");
        lord.addClass(post, "opPost");
    }
    perm.style.display = "";
    var anumber = lord.node("a");
    number.parentNode.insertBefore(anumber, number);
    number.parentNode.removeChild(number);
    anumber.title = number.title;
    anumber.appendChild(number);
    anumber.href = "#" + res["number"];
    (function(pn) {
        if (postingEnabled) {
            anumber.onclick = function() {
                lord.insertPostNumber(pn);
                return false;
            };
        }
    })(res["number"]);
    var deleteButton = lord.nameOne("deleteButton", post);
    deleteButton.href = deleteButton.href.replace("%postNumber%", res["number"]);
    var hideButton = lord.nameOne("hideButton", post);
    hideButton.id = hideButton.id.replace("%postNumber%", res["number"]);
    hideButton.href = hideButton.href.replace("%postNumber%", res["number"]);
    var addFileButton = lord.nameOne("addFileButton", post);
    var editButton = lord.nameOne("editButton", post);
    var fixButton = lord.nameOne("fixButton", post);
    var unfixButton = lord.nameOne("unfixButton", post);
    var openButton = lord.nameOne("openButton", post);
    var closeButton = lord.nameOne("closeButton", post);
    var banButton = lord.nameOne("banButton", post);
    var downloadButton = lord.nameOne("downloadButton", post);
    var favButton = lord.nameOne("addToFavoritesButton", post);
    var rawText = lord.nameOne("rawText", post);
    var rawHtml = lord.nameOne("rawHtml", post);
    if (res["rawHtml"])
        rawHtml.value = "true";
    lord.nameOne("draft", post).value = res["draft"];
    if ((moder || res["draft"]) && (res["files"].length < +lord.text("maxFileCount"))) {
        addFileButton.href = addFileButton.href.replace("%postNumber%", res["number"]);
    } else {
        addFileButton.parentNode.removeChild(addFileButton);
    }
    if (moder || res["draft"]) {
        editButton.href = editButton.href.replace("%postNumber%", res["number"]);
        rawText.value = res["rawPostText"];
        lord.nameOne("email", post).value = res["email"];
        lord.nameOne("name", post).value = res["rawName"];
        lord.nameOne("subject", post).value = res["rawSubject"];
    } else {
        editButton.parentNode.removeChild(editButton);
    }
    if (res["number"] != res["threadNumber"] || !inp || +inp.value !== res["threadNumber"])
        downloadButton.parentNode.removeChild(downloadButton);
    if (res["number"] == res["threadNumber"]) {
        var fav = lord.getLocalObject("favoriteThreads", {});
        if (fav.hasOwnProperty(boardName + "/" + res["number"])) {
            var img = lord.queryOne("img", favButton);
            img.title = lord.text("removeFromFavoritesText");
            img.src = img.src.replace("favorite.png", "favorite_active.png");
            favButton.onclick = lord.removeThreadFromFavorites.bind(lord, boardName, res["number"]);
        } else {
            favButton.onclick = lord.addThreadToFavorites.bind(lord, boardName, res["number"]);
        }
    } else {
        favButton.parentNode.removeChild(favButton);
    }
    if (!moder) {
        fixButton.parentNode.removeChild(fixButton);
        unfixButton.parentNode.removeChild(unfixButton);
        openButton.parentNode.removeChild(openButton);
        closeButton.parentNode.removeChild(closeButton);
        banButton.parentNode.removeChild(banButton);
        return post;
    }
    var toThread = lord.nameOne("toThread", post);
    if (res["number"] === res["threadNumber"]) {
        if (!!res["fixed"]) {
            unfixButton.style.display = "";
            unfixButton.href = unfixButton.href.replace("%postNumber%", res["number"]);
            fixButton.parentNode.removeChild(fixButton);
        } else {
            fixButton.style.display = "";
            fixButton.href = fixButton.href.replace("%postNumber%", res["number"]);
            unfixButton.parentNode.removeChild(unfixButton);
        }
        if (!!res["closed"]) {
            openButton.style.display = "";
            openButton.href = openButton.href.replace("%postNumber%", res["number"]);
            closeButton.parentNode.removeChild(closeButton);
        } else {
            closeButton.style.display = "";
            closeButton.href = closeButton.href.replace("%postNumber%", res["number"]);
            openButton.parentNode.removeChild(openButton);
        }
        if (!inp) {
            toThread.style.display = "";
            var toThreadLink = lord.nameOne("toThreadLink", post);
            toThreadLink.href = toThreadLink.href.replace("%postNumber%", res["number"]);
        } else {
            toThread.parentNode.removeChild(toThread);
        }
    } else {
        fixButton.parentNode.removeChild(fixButton);
        unfixButton.parentNode.removeChild(unfixButton);
        openButton.parentNode.removeChild(openButton);
        closeButton.parentNode.removeChild(closeButton);
        toThread.parentNode.removeChild(toThread);
    }
    banButton.href = banButton.href.replace("%postNumber%", res["number"]);
    return post;
};

lord.updatePost = function(boardName, postNumber, post) {
    postNumber = +postNumber;
    if (!boardName || !post || isNaN(postNumber) || postNumber <= 0)
        return;
    lord.ajaxRequest("get_post", [boardName, postNumber], lord.RpcGetPostId, function(res) {
        var newPost = lord.createPostNode(res, true);
        if (!newPost)
            return;
        var postLimit = lord.nameOne("postLimit", post);
        var bumpLimit = lord.nameOne("bumpLimit", post);
        if (!!postLimit || !!bumpLimit) {
            var postHeader = lord.queryOne(".postHeader", newPost);
            if (!!postLimit)
                postHeader.appendChild(postLimit.cloneNode(true));
            if (!!bumpLimit)
                postHeader.appendChild(bumpLimit.cloneNode(true));
        }
        post.parentNode.replaceChild(newPost, post);
        lord.postNodeInserted(newPost);
    });
};

lord.clearFileInput = function(div) {
    var preview = div.querySelector("img");
    if (!!preview && div == preview.parentNode)
        preview.src = "/" + lord.text("sitePathPrefix") + "img/addfile.png";
    var span = div.querySelector("span");
    if (!!span && div == span.parentNode) {
        while (span.firstChild)
            span.removeChild(span.firstChild);
    }
    if (div.parentNode)
        lord.removeFileHash(div);
    div.fileHash = null;
};

lord.readableSize = function(sz) {
    sz = +sz;
    if (isNaN(sz))
        return "";
    if (sz / 1024 >= 1) {
        sz /= 1024;
        if (sz / 1024 >= 1) {
            sz = (sz / 1024).toFixed(1);
            sz += " " + lord.text("megabytesText");
        } else {
            sz = sz.toFixed(1);
            sz += " " + lord.text("kilobytesText");
        }
    } else {
        sz = sz.toString();
        sz += " " + lord.text("bytesText");
    }
    return sz;
};

lord.getFileHashes = function(div) {
    var parent = div.parentNode.parentNode;
    var fhs = parent.querySelector("[name='fileHashes']");
    if (fhs)
        return fhs;
    return parent.parentNode.parentNode.parentNode.querySelector("[name='fileHashes']");
};

lord.getAdditionalCount = function(el) {
    if (!el)    
        return 0;
    el = el.parentNode;
    if (!el)    
        return 0;
    el = el.parentNode;
    if (!el)    
        return 0;
    el = el.parentNode;
    if (!el)    
        return 0;
    el = el.parentNode;
    if (!el)    
        return 0;
    el = lord.nameOne("additionalCount", el);
    return el ? el.value : 0;
};

lord.hideImage = function() {
    if (!!lord.img) {
        if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
            lord.img.pause();
            lord.img.load();
        }
        lord.img.style.display = "none";
        lord.img = null;
        lord.query(".leafButton").forEach(function(a) {
            a.style.display = "none";
        });
    }
};

lord.globalOnclick = function(e) {
    if (!!e.button)
        return;
    var t = e.target;
    if (!!t && !!lord.img && t == lord.img)
        return;
    while (!!t) {
        if (t.tagName === "A" && (!!t.onclick || !!t.onmousedown || !!t.href))
            return;
        t = t.parentNode;
    }
    lord.hideImage();
};

lord.initFiles = function() {
    if (!!lord.files)
        return;
    lord.files = [];
    lord.filesMap = {};
    lord.query(".postFileFile").forEach(function(div) {
        var href = lord.queryOne("a", div).href;
        var type = lord.nameOne("type", div).value;
        if ("application/pdf" == type)
            return;
        if (lord.getLocalObject("leafThroughImagesOnly", false) && !lord.isImageType(type))
            return;
        lord.files.push({
            "href": href,
            "type": type,
            "sizeX": lord.nameOne("sizeX", div).value,
            "sizeY": lord.nameOne("sizeY", div).value
        });
        lord.filesMap[href] = lord.files.length - 1;
    });
};

lord.nextOrPreviousFile = function(previous) {
    if (!lord.img || !lord.files || !lord.filesMap || lord.files.length < 1)
        return null;
    var href = lord.img.src;
    if (!href)
        href = lord.queryOne("source", lord.img).src;
    if (!href)
        return null;
    var ind = lord.filesMap[href];
    if (ind < 0)
        return null;
    if (!!previous)
        return lord.files[(ind > 0) ? (ind - 1) : (lord.files.length - 1)];
    else
        return lord.files[(ind < lord.files.length - 1) ? (ind + 1) : 0];
};

lord.addYoutubeButton = function(post) {
    if (!post)
        return;
    var key = lord.text("youtubeApiKey");
    if (!key)
        return;
    var q = "a[href^='http://youtube.com'], a[href^='https://youtube.com'], "
        + "a[href^='http://www.youtube.com'], a[href^='https://www.youtube.com']";
    lord.query(q, post).forEach(function(link) {
        if (link.href.replace("v=", "") == link.href)
            return;
        var img = lord.node("img");
        img.src = "https://youtube.com/favicon.ico";
        img.title = "YouTube";
        link.parentNode.insertBefore(img, link);
        link.parentNode.insertBefore(lord.node("text", " "), link);
        var a = lord.node("a");
        lord.addClass(a, "expandCollapse");
        a.lordExpanded = false;
        (function (a, link) {
            var videoId = link.href.split("v=").pop();
            videoId = videoId.match(/[a-zA-Z0-9_\-]{11}/)[0];
            var xhr = new XMLHttpRequest();
            xhr.open("get", "https://www.googleapis.com/youtube/v3/videos?id=" + videoId + "&key=" + key
                + "&part=snippet");
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4) {
                    if (xhr.status != 200)
                        return;
                    var response;
                    try {
                        response = JSON.parse(xhr.responseText);
                    } catch (ex) {
                        return;
                    }
                    var title = response.items[0].snippet.title;
                    link.replaceChild(lord.node("text", title), link.firstChild);
                }
            };
            xhr.send(null);
            a.onclick = function() {
                if (a.lordExpanded) {
                    a.parentNode.removeChild(a.nextSibling);
                    a.parentNode.removeChild(a.nextSibling);
                    a.replaceChild(lord.node("text", "[" + lord.text("expandVideoText") + "]"), a.childNodes[0]);
                    lord.removeClass(a.parentNode, "expand");
                } else {
                    lord.addClass(a.parentNode, "expand");
                    var iframe = lord.node("iframe");
                    iframe.src = "https://youtube.com/embed/" + videoId + "?autoplay=1";
                    iframe.allowfullscreen = true;
                    iframe.frameborder = "0px";
                    iframe.height = "360";
                    iframe.width = "640";
                    iframe.display = "block";
                    var parent = a.parentNode;
                    var el = a.nextSibling;
                    if (el) {
                        parent.insertBefore(lord.node("br"), el);
                        parent.insertBefore(iframe, el);
                    }
                    else {
                        parent.appendChild(lord.node("br"));
                        parent.appendChild(iframe);
                    }
                    a.replaceChild(lord.node("text", "[" + lord.text("collapseVideoText") + "]"), a.childNodes[0]);
                }
                a.lordExpanded = !a.lordExpanded;
            };
        })(a, link);
        a.appendChild(lord.node("text", "[" + lord.text("expandVideoText") + "]"));
        var el = link.nextSibling;
        var parent = link.parentNode;
        if (el) {
            parent.insertBefore(lord.node("text", " "), el);
            parent.insertBefore(a, el);
        } else {
            parent.appendChild(lord.node("text", " "));
            parent.appendChild(a);
        }
    });
};

lord.tryHidePost = function(post, list) {
    if (!post)
        return;
    var postNumber = post.id.replace("post", "");
    if (isNaN(+postNumber))
        return;
    var boardName = lord.text("currentBoardName");
    if (!list)
        list = lord.getLocalObject("hiddenPosts", {});
    //TODO: Custom hide rules
    if (!list[boardName + "/" + postNumber])
        return;
    lord.addClass(post, "hiddenPost");
    var thread = lord.id("thread" + postNumber);
    if (!thread)
        return;
    lord.addClass(thread, "hiddenThread");
    lord.addClass(lord.id("threadOmitted" + postNumber), "hiddenPosts");
    lord.addClass(lord.id("threadPosts" + postNumber), "hiddenPosts");
    lord.strikeOutHiddenPostLinks();
};

lord.postNodeInserted = function(post) {
    if (!post)
        return;
    if (lord.getLocalObject("showYoutubeVideosTitles", true))
        lord.addYoutubeButton(post);
    lord.tryHidePost(post);
    if (!!lord.getLocalObject("strikeOutHiddenPostLinks", true))
        lord.strikeOutHiddenPostLinks(post);
    var lastPostNumbers = lord.getLocalObject("lastPostNumbers", {});
    lastPostNumbers[lord.text("currentBoardName")] = +post.id.replace("post", "");
    lord.setLocalObject("lastPostNumbers", lastPostNumbers);
    lord.files = null;
    lord.filesMap = null;
    lord.initFiles();
};

lord.showPasswordDialog = function(title, label, callback) {
    var div = lord.node("div");
    var input = lord.node("input");
    input.type = "password";
    lord.addClass(input, "input");
    input.maxlength = 150;
    input.size = 30;
    div.appendChild(input);
    var sw = lord.node("input");
    sw.type = "checkbox";
    lord.addClass(sw, "checkbox");
    sw.title = lord.text("showPasswordText");
    sw.onclick = function() {
        if (input.type === "password")
            input.type = "text";
        else if (input.type === "text")
            input.type = "password";
    };
    div.appendChild(sw);
    lord.showDialog(title, label, div, function() {
        callback(input.value);
    }, function() {
        input.focus();
    });
};

lord.showHidePostForm = function(position) {
    lord.removeQuickReply();
    var postForm = lord.id("postForm");
    var theButton = lord.id("showHidePostFormButton" + position);
    if (lord.postForm.visibility[position]) {
        theButton.innerHTML = lord.text("showPostFormText");
        lord.id("hiddenPostForm").appendChild(postForm);
        lord.postForm.last = "";
    } else {
        var p = ("Top" === position) ? "Bottom" : "Top";
        if (lord.postForm.visibility[p])
            lord.showHidePostForm(p);
        theButton.innerHTML = lord.text("hidePostFormText");
        lord.id("createActionContainer" + position).appendChild(postForm);
    }
    lord.postForm.visibility[position] = !lord.postForm.visibility[position];
};

lord.switchShowTripcode = function() {
    var postForm = lord.id("postForm");
    var sw = lord.nameOne("tripcode", postForm);
    if (sw.checked)
        lord.setLocalObject("showTripcode", true);
    else
        lord.removeLocalObject("showTripcode");
};

lord.countSymbols = function(textarea) {
    if (!textarea)
        return;
    var span = lord.queryOne(".symbolCounter", textarea.parentNode);
    span = lord.nameOne("used", span);
    if (span.childNodes.length > 0)
        span.removeChild(span.childNodes[0]);
    span.appendChild(lord.node("text", textarea.value.length.toString()));
};

lord.deletePost = function(boardName, postNumber, fromThread) {
    if (!boardName || isNaN(+postNumber))
        return;
    var title = lord.text("enterPasswordTitle");
    var label = lord.text("enterPasswordText");
    lord.showPasswordDialog(title, label, function(pwd) {
        if (null === pwd)
            return;
        if (pwd.length < 1) {
            if (!lord.getCookie("hashpass"))
                return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
        } else if (!lord.isHashpass(pwd)) {
            pwd = lord.toHashpass(pwd);
        }
        lord.ajaxRequest("delete_post", [boardName, +postNumber, pwd], lord.RpcDeletePostId, function(res) {
            var post = lord.id("post" + postNumber);
            if (!post) {
                if (!!fromThread) {
                    var suffix = "thread/" + postNumber + ".html";
                    window.location.href = window.location.href.replace(suffix, "").split("#").shift();
                } else {
                    lord.reloadPage();
                }
                return;
            } else if (lord.hasClass(post, "opPost")) {
                var suffix = "thread/" + postNumber + ".html";
                window.location.href = window.location.href.replace(suffix, "").split("#").shift();
            } else {
                post.parentNode.removeChild(post);
                lord.removeReferences(postNumber);
                var postLinks = lord.query("a");
                if (!!postLinks) {
                    for (var i = 0; i < postLinks.length; ++i) {
                        var link = postLinks[i];
                        if (("&gt;&gt;" + postNumber) !== link.innerHTML)
                            continue;
                        var text = link.innerHTML.replace("&gt;&gt;", ">>");
                        link.parentNode.replaceChild(lord.node("text", text), link);
                    }
                }
                if (!!lord.postPreviews[boardName + "/" + postNumber])
                    delete lord.postPreviews[boardName + "/" + postNumber];
            }
        });
    });
};

lord.setThreadFixed = function(boardName, postNumber, fixed) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!lord.getCookie("hashpass"))
        return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
    lord.ajaxRequest("set_thread_fixed", [boardName, +postNumber, !!fixed], lord.RpcSetThreadFixedId, lord.reloadPage);
};

lord.setThreadOpened = function(boardName, postNumber, opened) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!lord.getCookie("hashpass"))
        return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
    lord.ajaxRequest("set_thread_opened", [boardName, +postNumber, !!opened], lord.RpcSetThreadOpenedId, lord.reloadPage);
};

lord.banUser = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var title = lord.text("banUserText");
    var div = lord.node("div");
    var div1 = lord.node("div");
    div1.appendChild(lord.node("text", lord.text("boardLabelText")));
    var selBoard = lord.id("availableBoardsSelect").cloneNode(true);
    selBoard.style.display = "block";
    div1.appendChild(selBoard);
    div.appendChild(div1);
    var div2 = lord.node("div");
    div2.appendChild(lord.node("text", lord.text("banLevelLabelText")));
    var selLevel = lord.id("banLevelsSelect").cloneNode(true);
    selLevel.style.display = "block";
    div2.appendChild(selLevel);
    div.appendChild(div2);
    var div3 = lord.node("div");
    div3.appendChild(lord.node("text", lord.text("banReasonLabelText")));
    var inputReason = lord.node("input");
    inputReason.type = "text";
    lord.addClass(inputReason, "input");
    div3.appendChild(inputReason);
    div.appendChild(div3);
    var div4 = lord.node("div");
    div4.appendChild(lord.node("text", lord.text("banExpiresLabelText")));
    var inputExpires = lord.node("input");
    inputExpires.type = "text";
    inputExpires.placeholder = "dd.MM.yyyy:hh";
    lord.addClass(inputExpires, "input");
    div4.appendChild(inputExpires);
    div.appendChild(div4);
    lord.showDialog(title, null, div, function() {
        var params = {
            "boardName": boardName,
            "postNumber": +postNumber,
            "board": selBoard.options[selBoard.selectedIndex].value,
            "level": +selLevel.options[selLevel.selectedIndex].value,
            "reason": inputReason.value,
            "expires": inputExpires.value
        };
        lord.ajaxRequest("ban_user", [params], lord.RpcBanUserId, function(res) {
            lord.reloadPage();
        });
    });
};

lord.insertPostNumber = function(postNumber) {
    try {
        var field = lord.nameOne("text", lord.id("postForm"));
        var value = ">>" + postNumber + "\n";
        if (document.selection) {
            var sel = document.selection.createRange();
            sel.text = value;
        } else if (field.selectionStart || field.selectionStart == "0") {
            var startPos = field.selectionStart;
            var endPos = field.selectionEnd;
            field.value = field.value.substring(0, startPos) + value + field.value.substring(endPos);
            var pos = ((startPos < endPos) ? startPos : endPos) + value.length;
            field.setSelectionRange(pos, pos);
        } else {
            field.value += value;
        }
        field.focus();
    } catch (ex) {
        //Do nothing
    }
};

lord.quickReply = function(postNumber) {
    var postForm = lord.id("postForm");
    if (lord.postForm.quickReply) {
        var prev = postForm.previousSibling;
        prev = prev ? prev.id.replace("post", "") : 0;
        lord.removeQuickReply();
        if (prev == postNumber)
            return;
    }
    postNumber = +postNumber;
    if (isNaN(postNumber) || postNumber <= 0)
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var parent = post.parentNode;
    var thread = lord.nameOne("thread", postForm);
    if (!thread) {
        var inp = lord.node("input");
        inp.type = "hidden";
        inp.name = "thread";
        inp.value = parent.id.replace("threadPosts", "").replace("thread", "");
        postForm.appendChild(inp);
        postForm.action = postForm.action.replace("create_thread", "create_post");
    }
    ["Top", "Bottom"].forEach(function(pos) {
        if (lord.postForm.visibility[pos]) {
            lord.showHidePostForm(pos);
            lord.postForm.last = pos;
        }
    });
    if (post.nextSibling)
        parent.insertBefore(postForm, post.nextSibling);
    else
        parent.appendChild(postForm);
    lord.insertPostNumber(postNumber);
    lord.postForm.quickReply = true;
};

lord.addFile = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var title = lord.text("addFileText");
    var div = lord.id("addFileTemplate").cloneNode(true);
    var form = lord.queryOne("form", div);
    div.id = "";
    div.style.display = "";
    lord.nameOne("additionalCount", div).value = lord.query(".postFile", post).length;
    lord.showDialog(title, null, div, function() {
        if (!lord.getCookie("hashpass"))
            return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
        lord.nameOne("postNumber", form).value = postNumber;
        var formData = new FormData(form);
        lord.query(".postformFile", form).forEach(function(div) {
            if (div.droppedFile)
                formData.append("file", div.droppedFile);
        });
        var xhr = new XMLHttpRequest();
        xhr.open("POST", form.action);
        var progress = lord.node("progress");
        lord.addClass(progress, "progressBlocking");
        progress.max = 100;
        progress.value = 0;
        document.body.appendChild(progress);
        lord.toCenter(progress, progress.offsetWidth, progress.offsetHeight);
        xhr.upload.onprogress = function(e) {
            progress.value = Math.floor(100 * (e.loaded / e.total));
        };
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    document.body.removeChild(progress);
                    var response = xhr.responseText;
                    var err = response.error;
                    if (!!err)
                        return lord.showPopup(err, {type: "critical"});
                    lord.updatePost(boardName, postNumber, post);
                } else {
                    document.body.removeChild(progress);
                    lord.showPopup(lord.text("ajaxErrorText") + " " + xhr.status, {type: "critical"});
                }
            }
        };
        xhr.send(formData);
        return false;
    });
};

lord.editPost = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var stage2 = function(boardName, postNumber, post, rawPostText) {
        var title = lord.text("editPostText");
        var form = lord.id("editPostTemplate").cloneNode(true);
        form.id = "";
        form.style.display = "";
        var email = lord.nameOne("email", form);
        var name = lord.nameOne("name", form);
        var subject = lord.nameOne("subject", form);
        var text = lord.nameOne("text", form);
        var used = lord.queryOne(".symbolCounter", form);
        used = lord.nameOne("used", used);
        email.value = lord.nameOne("email", post).value;
        name.value = lord.nameOne("name", post).value;
        subject.value = lord.nameOne("subject", post).value;
        text.appendChild(lord.node("text", rawPostText));
        used.appendChild(lord.node("text", text.value.length.toString()));
        var moder = (lord.text("moder") === "true");
        var draftField = lord.nameOne("draft", form);
        var rawField = lord.nameOne("raw", form);
        if (!!draftField) {
            if (lord.nameOne("draft", post).value == "true")
                draftField.checked = true;
            else
                draftField.parentNode.parentNode.style.display = "none";
        }
        if (!!rawField && lord.nameOne("rawHtml", post).value == "true")
            rawField.checked = true;
        if (lord.customEditFormSet)
            lord.customEditFormSet(form, post, !!draftField, !!rawField);
        lord.showDialog(title, null, form, function() {
            var pwd = lord.nameOne("password", form).value;
            if (pwd.length < 1) {
                if (!lord.getCookie("hashpass"))
                    return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
            } else if (!lord.isHashpass(pwd)) {
                pwd = lord.toHashpass(pwd);
            }
            var params = {
                "boardName": boardName,
                "postNumber": +postNumber,
                "text": text.value,
                "email": email.value,
                "name": name.value,
                "subject": subject.value,
                "raw": !!rawField ? form.querySelector("[name='raw']").checked : false,
                "draft": !!draftField ? draftField.checked : false,
                "password": pwd,
                "userData": null
            };
            if (lord.customEditFormGet)
                params["userData"] = lord.customEditFormGet(form, params);
            lord.ajaxRequest("edit_post", [params], lord.RpcEditPostId, function() {
                lord.updatePost(boardName, postNumber, post);
            });
        });
    };
    var rawPostText = lord.nameOne("rawText", post);
    if (rawPostText) {
        stage2(boardName, postNumber, post, rawPostText.value);
    } else {
        lord.ajaxRequest("get_post", [boardName, +postNumber], lord.RpcGetPostId, function(res) {
            stage2(boardName, postNumber, post, res["rawPostText"]);
        });
    }
};

lord.setPostHidden = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var thread = lord.id("thread" + postNumber);
    var list = lord.getLocalObject("hiddenPosts", {});
    var hidden = lord.hasClass(post, "hiddenPost");
    var f = !hidden ? lord.addClass : lord.removeClass;
    f(post, "hiddenPost");
    if (thread) {
        var omitted = lord.id("threadOmitted" + postNumber);
        var posts = lord.id("threadPosts" + postNumber);
        f(thread, "hiddenThread");
        f(omitted, "hiddenPosts");
        f(posts, "hiddenPosts");
    }
    if (!hidden)
        list[boardName + "/" + postNumber] = {};
    else if (list[boardName + "/" + postNumber])
        delete list[boardName + "/" + postNumber];          
    lord.setLocalObject("hiddenPosts", list);
    lord.strikeOutHiddenPostLinks();
};

lord.deleteFile = function(boardName, postNumber, fileName) {
    if (!boardName || isNaN(+postNumber) || !fileName)
        return;
    var title = lord.text("enterPasswordTitle");
    var label = lord.text("enterPasswordText");
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    lord.showPasswordDialog(title, label, function(pwd) {
        if (null === pwd)
            return;
        if (pwd.length < 1) {
            if (!lord.getCookie("hashpass"))
                return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
        } else if (!lord.isHashpass(pwd)) {
            pwd = lord.toHashpass(pwd);
        }
        lord.ajaxRequest("delete_file", [boardName, fileName, pwd], lord.RpcDeleteFileId, function() {
            lord.updatePost(boardName, postNumber, post);
        });
    });
};

lord.editAudioTags = function(boardName, postNumber, fileName) {
    if (!boardName || isNaN(+postNumber) || !fileName)
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var dlgTitle = lord.text("editAudioTagsText");
    var table = lord.id("editAudioTagsTemplate").cloneNode(true);
    table.id = "";
    table.style.display = "";
    var f = lord.id("file" + fileName);
    var album = lord.nameOne("album", table);
    var artist = lord.nameOne("artist", table);
    var title = lord.nameOne("title", table);
    var year = lord.nameOne("year", table);
    album.value = lord.nameOne("audioTagAlbum", f).value;
    artist.value = lord.nameOne("audioTagArtist", f).value;
    title.value = lord.nameOne("audioTagTitle", f).value;
    year.value = lord.nameOne("audioTagYear", f).value;
    lord.showDialog(dlgTitle, null, table, function() {
        var pwd = lord.nameOne("password", table).value;
        if (pwd.length < 1) {
            if (!lord.getCookie("hashpass"))
                return lord.showPopup(lord.text("notLoggedInText"), {type: "critical"});
        } else if (!lord.isHashpass(pwd)) {
            pwd = lord.toHashpass(pwd);
        }
        var tags = {
            "album": album.value,
            "artist": artist.value,
            "title": title.value,
            "year": year.value
        };
        lord.ajaxRequest("edit_audio_tags", [boardName, fileName, pwd, tags], lord.RpcEditAudioTagsId, function() {
            lord.updatePost(boardName, postNumber, post);
        });
    });
};

lord.addToPlaylist = function(boardName, fileName) {
    if (!boardName || !fileName)
        return;
    var tracks = lord.getLocalObject("playlist/tracks", {});
    if (tracks[boardName + "/" + fileName])
        return;
    tracks[boardName + "/" + fileName] = {};
    lord.setLocalObject("playlist/tracks", tracks);
};

lord.viewPostStage2 = function(link, boardName, postNumber, post) {
    post.onmouseout = function(event) {
        var next = post;
        while (!!next) {
            var list = lord.traverseChildren(next);
            var e = event.toElement || event.relatedTarget;
            if (list.indexOf(e) >= 0)
                return;
            next = next.nextPostPreview;
        }
        if (!!post.parentNode)
            post.parentNode.removeChild(post);
        if (post.previousPostPreview)
            post.previousPostPreview.onmouseout(event);
    };
    post.onmouseover = function(event) {
        post.mustHide = false;
    };
    if (!lord.postPreviews[boardName + "/" + postNumber])
        lord.postPreviews[boardName + "/" + postNumber] = post;
    else
        post.style.display = "";
    post.previousPostPreview = lord.lastPostPreview;
    if (!!lord.lastPostPreview)
        lord.lastPostPreview.nextPostPreview = post;
    lord.lastPostPreview = post;
    post.mustHide = true;
    if (!!lord.lastPostPreviewTimer) {
        clearTimeout(lord.lastPostPreviewTimer);
        lord.lastPostPreviewTimer = null;
    }
    document.body.appendChild(post);
    post.style.position = "absolute";
    var doc = document.documentElement;
    var coords = link.getBoundingClientRect();
    var linkCenter = coords.left + (coords.right - coords.left) / 2;
    if (linkCenter < 0.6 * doc.clientWidth) {
        post.style.maxWidth = doc.clientWidth - linkCenter + "px";
        post.style.left = linkCenter + "px";
    } else {
        post.style.maxWidth = linkCenter + "px";
        post.style.left = linkCenter - post.scrollWidth + "px";
    }
    var scrollTop = doc.scrollTop;
    if (!scrollTop) //NOTE: Workaround for Chrome/Safari. I really HATE you, HTML/CSS/JS!
        scrollTop = document.body.scrollTop;
    post.style.top = (doc.clientHeight - coords.bottom >= post.scrollHeight)
        ? (scrollTop + coords.bottom - 4 + "px")
        : (scrollTop + coords.top - post.scrollHeight - 4 + "px");
    post.style.zIndex = 9001;
};

lord.viewPost = function(link, boardName, postNumber) {
    postNumber = +postNumber;
    if (!link || !boardName || isNaN(postNumber) || postNumber <= 0)
        return;
    var currentBoardName = lord.text("currentBoardName");
    var post = null;
    if (boardName === currentBoardName)
        post = lord.id("post" + postNumber);
    if (!post)
        post = lord.postPreviews[boardName + "/" + postNumber];
    if (!post) {
        lord.ajaxRequest("get_post", [boardName, +postNumber], lord.RpcGetPostId, function(res) {
            post = lord.createPostNode(res, false, boardName);
            if (!post)
                return;
            post["fromAjax"] = true;
            lord.viewPostStage2(link, boardName, postNumber, post);
        });
    } else {
        var fromAjax = !!post.fromAjax;
        if (fromAjax)
            var ajaxFiles = post.ajaxFiles;
        post = post.cloneNode(true);
        lord.removeClass(post, "newPost selectedPost");
        lord.addClass(post, "temporary");
        if (fromAjax) {
            post.addEventListener("click", function(e) {
                var img = e.target;
                if (img.tagName != "IMG")
                    return;
                if (!ajaxFiles)
                    return;
                var ajaxFile = ajaxFiles[img.src.split("/").pop()];
                if (!ajaxFile)
                    return;
                var ind = img.src.lastIndexOf("/");
                var href = img.src.substring(0, ind) + "/" + ajaxFile["sourceName"];
                if (lord.showImage(href, ajaxFile["type"], ajaxFile["sizeX"], ajaxFile["sizeY"]) === false)
                    e.preventDefault();
            });
        }
        lord.removeClass(post, "opPost");
        lord.addClass(post, "post");
        var list = lord.traverseChildren(post);
        for (var i = 0; i < list.length; ++i) {
            switch (list[i].name) {
            case "editButton":
            case "fixButton":
            case "closeButton":
            case "banButton":
            case "deleteButton":
            case "hideButton":
                list[i].parentNode.removeChild(list[i]);
                break;
            default:
                break;
            }
            list[i].id = "";
        }
        lord.viewPostStage2(link, boardName, postNumber, post);
    }
};

lord.noViewPost = function() {
    lord.lastPostPreviewTimer = setTimeout(function() {
        if (!lord.lastPostPreview)
            return;
        if (!!lord.lastPostPreview.mustHide && !!lord.lastPostPreview.parentNode)
            lord.lastPostPreview.parentNode.removeChild(lord.lastPostPreview);
    }, 500);
};

lord.fileDragOver = function(e, div) {
    e.preventDefault();
    lord.addClass(div, "drag");
    return false;
};

lord.fileDragLeave = function(e, div) {
    e.preventDefault();
    lord.removeClass(div, "drag");
    return false;
};

lord.removeFileHash = function(div) {
    if (!div)
        return;
    if (!div.fileHash)
        return;
    var fileHashes = lord.getFileHashes(div);
    var val = fileHashes.value.replace("," + div.fileHash, "");
    if (val === fileHashes.value)
        val = fileHashes.value.replace(div.fileHash + ",", "");
    if (val === fileHashes.value)
        val = fileHashes.value.replace(div.fileHash, "");
    fileHashes.value = val;
};

lord.fileAddedCommon = function(div, file) {
    if (!div || !file)
        return;
    var inp = lord.queryOne("input", div);
    if (!inp)
        return;
    div.querySelector("span").appendChild(lord.node("text", file.name + " (" + lord.readableSize(file.size) + ")"));
    lord.removeFileHash(div);
    var binaryReader = new FileReader();
    var prefix = lord.text("sitePathPrefix");
    binaryReader.onload = function(e) {
        var wordArray = CryptoJS.lib.WordArray.create(e.target.result);
        var currentBoardName = lord.text("currentBoardName");
        var fileHash = lord.toHashpass(wordArray);
        lord.ajaxRequest("get_file_existence", [currentBoardName, fileHash], lord.RpcGetFileExistenceId, function(res) {
            if (!res)
                return;
            var img = lord.node("img");
            img.src = "/" + prefix + "img/storage.png";
            img.title = lord.text("fileExistsOnServerText");
            div.querySelector("span").appendChild(lord.node("text", " "));
            div.querySelector("span").appendChild(img);
            var fileHashes = lord.getFileHashes(div);
            if (fileHashes.value.indexOf(fileHash) < 0)
                fileHashes.value = fileHashes.value + (fileHashes.value.length > 0 ? "," : "") + fileHash;
            var f = inp.onchange;
            delete inp.onchange;
            inp.value = "";
            inp.onchange = f;
            div.fileHash = fileHash;
            if (div.droppedFile)
                delete div.droppedFile;
        });
    };
    if (lord.getLocalObject("checkFileExistence", true))
        binaryReader.readAsArrayBuffer(file);
    if (!!file.name.match(/\.(jpe?g|png|gif)$/i) && lord.getLocalObject("showAttachedFilePreview", true)) {
        var reader = new FileReader();
        reader.readAsDataURL(file);
        reader.onload = function(e) {
            div.querySelector("img").src = e.target.result;
        };
    } else if (!!file.name.match(/\.(jpe?g)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/jpeg_file.png";
    } else if (!!file.name.match(/\.(png)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/png_file.png";
    } else if (!!file.name.match(/\.(gif)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/gif_file.png";
    } else if (!!file.name.match(/\.(mp3)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/mp3_file.png";
    } else if (!!file.name.match(/\.(mp4)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/mp4_file.png";
    } else if (!!file.name.match(/\.(ogg|ogv)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/ogg_file.png";
    } else if (!!file.name.match(/\.(webm)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/webm_file.png";
    } else if (!!file.name.match(/\.(wav)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/wav_file.png";
    } else if (!!file.name.match(/\.(pdf)$/i)) {
        div.querySelector("img").src = "/" + prefix + "img/pdf_file.png";
    } else {
        div.querySelector("img").src = "/" + prefix + "img/file.png";
    }
    div.querySelector("a").style.display = "inline";
    var inpMax = lord.id("maxFileCount");
    if (!inpMax)
        return;
    var maxCount = +inpMax.value;
    var additionalCount = lord.getAdditionalCount(inp);
    if (additionalCount)
        maxCount -= +additionalCount;
    if (isNaN(maxCount))
        return;
    var parent = div.parentNode;
    if (parent.children.length >= maxCount)
        return;
    for (var i = 0; i < parent.children.length; ++i) {
        var c = parent.children[i];
        if (!c.fileHash && lord.queryOne("input", c).value === "" && !c.droppedFile)
            return;
        c.querySelector("a").style.display = "inline";
    }
    (function(div) {
        div = div.cloneNode(true);
        var span = lord.queryOne(".postformFileText", div);
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        lord.clearFileInput(div);
        parent.appendChild(div);
    })(div);
};

lord.fileDrop = function(e, div) {
    e.preventDefault();
    lord.removeClass(div, "drag");
    var inp = lord.queryOne("input", div);
    inp.parentNode.replaceChild(inp.cloneNode(true), inp);
    lord.clearFileInput(div);
    var file = e.dataTransfer.files[0];
    div.droppedFile = file;
    lord.fileAddedCommon(div, file);
    return false;
};

lord.fileSelected = function(current) {
    if (!current)
        return;
    var div = current.parentNode;
    if (div.droppedFile)
        delete div.droppedFile;
    if (current.value == "")
        return lord.removeFile(current);
    lord.clearFileInput(div);
    var file = current.files[0];
    lord.fileAddedCommon(div, file);
};

lord.removeFile = function(current) {
    if (!current)
        return;
    var div = current.parentNode;
    lord.removeFileHash(div);
    var parent = div.parentNode;
    parent.removeChild(div);
    lord.clearFileInput(div);
    if (div.droppedFile)
        delete div.droppedFile;
    if (parent.children.length > 1) {
        for (var i = 0; i < parent.children.length; ++i) {
            var c = parent.children[i];
            if (!c.fileHash && lord.queryOne("input", c).value === "" && !c.droppedFile)
                return;
        }
        var inp = lord.id("maxFileCount");
        if (!inp)
            return;
        var maxCount = +inp.value;
        var additionalCount = lord.getAdditionalCount(current);
        if (additionalCount)
            maxCount -= +additionalCount;
        if (isNaN(maxCount))
            return;
        if (parent.children.length >= maxCount)
            return;
        div = div.cloneNode(true);
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        parent.appendChild(div);
    }
    if (parent.children.length > 0) {
        var c = parent.children[0];
        if (!c.fileHash && lord.queryOne("input", c).value === "" && !c.droppedFile)
            c.querySelector("a").style.display = "none";
    } 
    if (parent.children.length < 1) {
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        parent.appendChild(div);
    }
};

lord.browseFile = function(e, div) {
    var inp = div.querySelector("input");
    if (!inp)
        return;
    var e = window.event || e;
    var a = e.target;
    while (!!a) {
        if (a.tagName === "A")
            return;
        a = a.parentNode;
    }
    inp.click();
};

lord.setPostformRulesVisible = function(visible) {
    var hide = !visible;
    lord.setCookie("hidePostformRules", hide, {
        "expires": lord.Billion, "path": "/"
    });
    lord.query(".postformRules > ul").forEach(function(ul) {
        ul.style.display = hide ? "none" : "";
    });
    var a = lord.queryOne(".postformRules > a");
    var aa = lord.node("a");
    lord.addClass(aa, "hidePostformRulesButton");
    aa.onclick = lord.setPostformRulesVisible.bind(lord, hide);
    aa.appendChild(lord.node("text", lord.text(hide ? "showPostformRulesText" : "hidePostformRulesText")));
    a.parentNode.replaceChild(aa, a);
};

lord.markup = function(tag) {
    var wrap = function(opTag, clTag) {
        if (!opTag || !clTag)
            return;
        try {
            var field = lord.nameOne("text", lord.id("postForm"));
            var pos = 0;
            if (field.selectionStart || field.selectionStart == "0") {
                var startPos = field.selectionStart;
                var endPos = field.selectionEnd;
                var selected = field.value.substring(startPos, endPos);
                var value = opTag + selected + clTag;
                field.value = field.value.substring(0, startPos) + value + field.value.substring(endPos);
                pos = ((startPos < endPos) ? startPos : endPos) + opTag.length;
                if (selected.length > 0)
                    pos += selected.length + clTag.length;
            } else {
                field.value += opTag + clTag;
                pos = field.value.length - clTag.length;
            }
            field.setSelectionRange(pos, pos);
            field.focus();
        } catch (ex) {
            //Do nothing
        }
    };
    switch (tag) {
    case "b":
    case "i":
    case "s":
    case "u":
    case "spoiler":
    case "sup":
    case "sub":
    case "url": {
        wrap("[" + tag + "]", "[/" + tag + "]");
        break;
    }
    case ">": {
        try {
            var field = lord.nameOne("text", lord.id("postForm"));
            var value = ">";
            var pos = 0;
            if (document.getSelection())
                value += document.getSelection().toString();
            if (field.selectionStart || field.selectionStart == "0") {
                var startPos = field.selectionStart;
                var endPos = field.selectionEnd;
                field.value = field.value.substring(0, startPos) + value + field.value.substring(endPos);
                pos = ((startPos < endPos) ? startPos : endPos) + value.length;
            } else {
                field.value += value;
            }
            field.setSelectionRange(pos, pos);
            field.focus();
        } catch (ex) {
            //Do nothing
        }
        break;
    }
    case "code": {
        var sel = lord.queryOne(".postformMarkup > span > [name='langSelect']");
        var lang = sel.options[sel.selectedIndex].value;
        wrap("[" + (("-" != lang) ? (tag + " lang=\"" + lang + "\"") : tag) + "]", "[/" + tag + "]");
        break;
    }
    default: {
        break;
    }
    }
};

lord.changeLastCodeLang = function() {
    var sel = lord.queryOne(".postformMarkup > span > [name='langSelect']");
    var lang = sel.options[sel.selectedIndex].value;
    lord.setLocalObject("lastCodeLang", lang);
};

lord.setPostformMarkupVisible = function(visible) {
    var hide = !visible;
    lord.setLocalObject("hidePostformMarkup", hide);
    lord.queryOne(".postformMarkup > span").style.display = hide ? "none" : "";
    var a = lord.queryOne(".postformMarkup > a");
    lord.removeChildren(a);
    a.appendChild(lord.node("text", lord.text(hide ? "showPostformMarkupText" : "hidePostformMarkupText")));
    a.onclick = lord.setPostformMarkupVisible.bind(lord, hide);
    return false;
};

lord.showImage = function(href, type, sizeHintX, sizeHintY) {
    lord.hideImage();
    if (!href || !type)
        return true;
    lord.img = lord.images[href];
    if (!!lord.img) {
        lord.setInitialScale(lord.img, sizeHintX, sizeHintY);
        lord.resetScale(lord.img);
        lord.img.style.display = "";
        lord.toCenter(lord.img, sizeHintX, sizeHintY);
        if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
            setTimeout(function() {
                lord.img.play();
            }, 500);
        }
        if (lord.getLocalObject("showLeafButtons", true)) {
            lord.query(".leafButton").forEach(function(a) {
                a.style.display = "";
            });
        }
        return false;
    }
    if (lord.isAudioType(type)) {
        sizeHintX = 400;
        lord.img = lord.node("audio");
        lord.img.width = sizeHintX + "px";
        lord.img.controls = "controls";
        lord.img.volume = lord.getLocalObject("defaultAudioVideoVolume", 100) / 100;
        var src = lord.node("source");
        src.src = href;
        src.type = type;
        lord.img.appendChild(src);
    } else if (lord.isImageType(type)) {
        if (!sizeHintX || !sizeHintY || sizeHintX <= 0 || sizeHintY <= 0)
            return true;
        lord.img = lord.node("img");
        lord.img.width = sizeHintX;
        lord.img.height = sizeHintY;
        lord.img.src = href;
    } else if (lord.isVideoType(type)) {
        lord.img = lord.node("video");
        lord.img.controls = "controls";
        lord.img.volume = lord.getLocalObject("defaultAudioVideoVolume", 100) / 100;
        var src = lord.node("source");
        src.src = href;
        src.type = type;
        lord.img.appendChild(src);
    }
    lord.img.fileType = type;
    lord.setInitialScale(lord.img, sizeHintX, sizeHintY);
    lord.resetScale(lord.img);
    lord.img.moving = false;
    lord.img.coord = {
        "x": 0,
        "y": 0
    };
    lord.img.initialCoord = {
        "x": 0,
        "y": 0
    };
    lord.addClass(lord.img, "movableImage");
    var wheelHandler = function(e) {
        var e = window.event || e; //Old IE support
        e.preventDefault();
        var delta = lord.getLocalObject("imageZoomSensitivity", 25);
        if ((e.wheelDelta || -e.detail) < 0)
            delta *= -1;
        if ((lord.img.scale + delta) <= 0)
            return;
        lord.img.scale += delta;
        lord.resetScale(lord.img);
    };
    if (lord.img.addEventListener) {
    	lord.img.addEventListener("mousewheel", wheelHandler, false); //IE9, Chrome, Safari, Opera
	    lord.img.addEventListener("DOMMouseScroll", wheelHandler, false); //Firefox
    } else {
        lord.img.attachEvent("onmousewheel", wheelHandler); //IE 6/7/8
    }
    if (lord.isImageType(type)) {
        lord.img.onmousedown = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            lord.img.moving = true;
            lord.img.coord.x = e.clientX;
            lord.img.coord.y = e.clientY;
            lord.img.initialCoord.x = e.clientX;
            lord.img.initialCoord.y = e.clientY;
        };
        lord.img.onmouseup = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            lord.img.moving = false;
            if (lord.img.initialCoord.x === e.clientX && lord.img.initialCoord.y === e.clientY) {
                if (lord.isAudioType(type) || lord.isVideoType(type)) {
                    lord.img.pause();
                    lord.img.currentTime = 0;
                }
                lord.img.style.display = "none";
                lord.query(".leafButton").forEach(function(a) {
                    a.style.display = "none";
                });
            }
        };
        lord.img.onmousemove = function(e) {
            if (!lord.img.moving)
                return;
            e.preventDefault();
            var dx = e.clientX - lord.img.coord.x;
            var dy = e.clientY - lord.img.coord.y;
            lord.img.style.left = (lord.img.offsetLeft + dx) + "px";
            lord.img.style.top = (lord.img.offsetTop + dy) + "px";
            lord.img.coord.x = e.clientX;
            lord.img.coord.y = e.clientY;
        };
    }
    document.body.appendChild(lord.img);
    lord.toCenter(lord.img, sizeHintX, sizeHintY);
    if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
        setTimeout(function() {
            lord.img.play();
        }, 500);
    }
    lord.images[href] = lord.img;
    if (lord.getLocalObject("showLeafButtons", true)) {
        lord.query(".leafButton").forEach(function(a) {
            a.style.display = "";
        });
    }
    return false;
};

lord.previousFile = function() {
    var f = lord.nextOrPreviousFile(true);
    if (!f)
        return;
    lord.showImage(f.href, f.type, f.sizeX, f.sizeY);
};

lord.nextFile = function() {
    var f = lord.nextOrPreviousFile(false);
    if (!f)
        return;
    lord.showImage(f.href, f.type, f.sizeX, f.sizeY);
};

lord.addThreadToFavorites = function(boardName, threadNumber, callback, callbackError) {
    threadNumber = +threadNumber;
    if (!boardName || isNaN(threadNumber)) {
        if (typeof callbackError == "function")
            callbackError();
        return false;
    }
    var fav = lord.getLocalObject("favoriteThreads", {});
    if (fav.hasOwnProperty(boardName + "/" + threadNumber)) {
        if (typeof callbackError == "function")
            callbackError();
        return false;
    }
    lord.ajaxRequest("get_new_posts", [boardName, threadNumber, 0], lord.RpcGetNewPostsId, function(res) {
        if (!res || res.length < 1) {
            if (typeof callbackError == "function")
                callbackError();
            return false;
        }
        var pn = res.pop()["number"];
        fav[boardName + "/" + threadNumber] = {
            "lastPostNumber": pn,
            "previousLastPostNumber": pn
        };
        var opPost = lord.id("post" + threadNumber);
        var btn = lord.nameOne("addToFavoritesButton", opPost);
        var img = lord.queryOne("img", btn);
        img.title = lord.text("removeFromFavoritesText");
        img.src = img.src.replace("favorite.png", "favorite_active.png");
        btn.onclick = lord.removeThreadFromFavorites.bind(lord, boardName, threadNumber);
        lord.setLocalObject("favoriteThreads", fav);
        if (typeof callback == "function")
            callback();
    });
    return false;
};

lord.complain = function() {
    lord.showPopup(lord.text("complainMessage"), {type: "critical"});
    if (!lord.complainVideo) {
        lord.complainVideo = lord.node("video");
        lord.complainVideo.style.display = "none";
        var src = lord.node("source");
        src.src = "/" + lord.text("sitePathPrefix") + "video/fail.webm";
        src.type = "video/webm";
        lord.complainVideo.appendChild(src);
        lord.complainVideo.volume = 0.5;
        document.body.appendChild(lord.complainVideo);
    }
    lord.complainVideo.play();
};

lord.submitted = function(event, form) {
    event.preventDefault();
    var btn = form.querySelector("[name='submit']");
    btn.disabled = true;
    btn.value = lord.text("postFormButtonSubmitSending") + " 0%";
    var resetButton = function() {
        btn.disabled = false;
        btn.value = lord.text("postFormButtonSubmit");
    };
    var formData = new FormData(form);
    lord.query(".postformFile", form).forEach(function(div) {
        if (div.droppedFile)
            formData.append("file", div.droppedFile);
    });
    var xhr = new XMLHttpRequest();
    xhr.open("POST", form.action);
    xhr.upload.onprogress = function(e) {
        var percent = Math.floor(100 * (e.loaded / e.total));
        if (100 == percent)
            btn.value = lord.text("postFormButtonSubmitWaiting");
        else
            btn.value = lord.text("postFormButtonSubmitSending") + " " + percent + "%";
    };
    xhr.onreadystatechange = function() {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                var response = xhr.responseText;
                var err = response.error;
                if (!!err) {
                    resetButton();
                    lord.showPopup(err, {type: "critical"});
                }
                lord.posted(response);
            } else {
                resetButton();
                var text = lord.text("ajaxErrorText") + " " + xhr.status;
                switch (+xhr.status) {
                case 413:
                    text = lord.text("error" + xhr.status + "Text");
                    break;
                default:
                    break;
                }
                lord.showPopup(text, {type: "critical"});
            }
        }
    };
    xhr.send(formData);
    return false;
};

lord.removeQuickReply = function() {
    if (!lord.postForm.quickReply)
        return;
    var postForm = lord.id("postForm");
    if (!lord.text("currentThreadNumber")) {
        postForm.removeChild(lord.nameOne("thread", postForm));
        postForm.action = postForm.action.replace("create_post", "create_thread");
    }
    lord.id("hiddenPostForm").appendChild(postForm);
    lord.postForm.quickReply = false;
    if (lord.postForm.last)
        lord.showHidePostForm(lord.postForm.last);
};

lord.resetPostForm = function() {
    var postForm = lord.id("postForm");
    postForm.reset();
    var divs = lord.query(".postformFile", postForm);
    for (var i = divs.length - 1; i >= 0; --i)
    lord.removeFile(lord.queryOne("a", divs[i]));
    if (lord.customResetForm)
        lord.customResetForm(postForm);
};

lord.posted = function(response) {
    var postForm = lord.id("postForm");
    var o = {};
    try {
        o = JSON.parse(response);
    } catch (ex) {
        //
    }
    var postNumber = o.postNumber;
    var threadNumber = o.threadNumber;
    var boardName = lord.text("currentBoardName");
    var currentThreadNumber = lord.text("currentThreadNumber");
    var resetButton = function() {
        var btn = postForm.querySelector("[name='submit']");
        btn.disabled = false;
        btn.value = lord.text("postFormButtonSubmit");
    };
    var f = function() {
        if (postNumber) {
            if (lord.postForm.quickReply && !currentThreadNumber) {
                var action = lord.getLocalObject("quickReplyAction", "goto_thread");
                if ("do_nothing" === action) {
                    //Do nothing
                } else if ("append_post" == action) {
                    var parent = postForm.parentNode;
                    if (!lord.hasClass(parent, "threadPosts")) {
                        parent = parent.nextSibling;
                        if (!parent.tagName)
                            parent = parent.nextSibling;
                    }
                    //NOTE: Yep, twice
                    if (!lord.hasClass(parent, "threadPosts")) {
                        parent = parent.nextSibling;
                        if (!parent.tagName)
                            parent = parent.nextSibling;
                    }
                    lord.ajaxRequest("get_post", [boardName, postNumber], lord.RpcGetPostId, function(res) {
                        var newPost = lord.createPostNode(res, true);
                        if (newPost) {
                            parent.appendChild(newPost, parent.lastChild);
                            lord.postNodeInserted(newPost);
                        }
                    });
                } else {
                    //The default
                    var href = window.location.href.split("#").shift();
                    href += "/thread/" + lord.nameOne("thread", postForm).value + ".html#" + postNumber;
                    window.location.href = href;
                    return;
                }
            }
            resetButton();
            lord.resetPostForm();
            if (currentThreadNumber)
                lord.updateThread(boardName, currentThreadNumber, true, lord.selectPost.bind(lord, postNumber));
            lord.removeQuickReply();
            lord.resetCaptcha();
        } else if (threadNumber) {
            var href = window.location.href.split("#").shift();
            window.location.href = href + (href.substring(href.length - 1) != "/" ? "/" : "") + "thread/" + threadNumber
                + ".html";
        } else {
            resetButton();
            var errmsg = o.errorMessage;
            var errdesc = o.errorDescription;
            var txt = (errmsg && errdesc) ? (errmsg + ": " + errdesc) : response.substring(0, 150);
            lord.showPopup(txt, {type: "critical"});
            lord.resetCaptcha();
        }
    };
    if (lord.getLocalObject("addToFavoritesOnReply", false) && (postNumber || threadNumber))
        lord.addThreadToFavorites(boardName, threadNumber || lord.nameOne("thread", postForm).value, f, f);
    else
        f();
};

lord.globalOnmouseover = function(e) {
    var a = e.target;
    if (a.tagName != "A" || a.innerHTML == a.innerHTML.replace("&gt;&gt;", ""))
        return;
    var pn = +a.href.split("#").pop();
    if (isNaN(pn) || pn <= 0)
        return;
    var bn = a.href.replace(/\/thread\/\d+\.html#\d+$/, "").split("/").pop();
    lord.viewPost(a, bn, pn);
};

lord.globalOnmouseout = function(e) {
    var a = e.target;
    if (a.tagName != "A" || a.innerHTML == a.innerHTML.replace("&gt;&gt;", ""))
        return;
    var pn = +a.href.split("#").pop();
    if (isNaN(pn) || pn <= 0)
        return;
    lord.noViewPost();
};

lord.strikeOutHiddenPostLinks = function(parent) {
    if (!parent)
        parent = document;
    var list = lord.getLocalObject("hiddenPosts", {});
    var cbn = lord.text("currentBoardName");
    lord.query("a", parent).forEach(function(a) {
        var m = a.textContent.match(/^>>(\/(.+)\/)?(\d+)$/);
        if (!m || !m.length || m.length < 3)
            return;
        var bn = m[2] ? m[2] : cbn;
        var pn = m[m.length - 1];
        if (list[bn + "/" + pn])
            lord.addClass(a, "hiddenPostLink");
        else
            lord.removeClass(a, "hiddenPostLink");
    });
};

lord.hotkey_previousPageImage = function() {
    if (!!lord.img) {
        lord.previousFile();
        return false;
    }
    if (lord.text("currentThreadNumber"))
        return;
    var curr = lord.queryOne(".pagesItem.currentPage");
    var list = lord.query(".pagesItem:not(.metaPage)");
    for (var i = 1; i < list.length; ++i) {
        if (curr == list[i]) {
            window.location.href = lord.queryOne("a", list[i - 1]).href;
            return false;
        }
    }
};

lord.hotkey_nextPageImage = function() {
    if (!!lord.img) {
        lord.nextFile();
        return false;
    }
    if (lord.text("currentThreadNumber"))
        return;
    var curr = lord.queryOne(".pagesItem.currentPage");
    var list = lord.query(".pagesItem:not(.metaPage)");
    for (var i = 0; i < list.length - 1; ++i) {
        if (curr == list[i]) {
            window.location.href = lord.queryOne("a", list[i + 1]).href;
            return false;
        }
    }
};

lord.previousNextThreadPostCommon = function(next, post) {
    var list = null;
    var f = function(list, i) {
        if (next && i < list.length - 1)
            return i + 1;
        else if (!next && i > 0)
            return i - 1;
        return i;
    };
    if (!post && !lord.text("currentThreadNumber")) {
        list = lord.query(".opPost");
        for (var i = 0; i < list.length; ++i) {
            if (lord.isInViewport(list[i])) {
                i = f(list, i);
                window.location.hash = list[i].id.replace("post", "");
                return false;
            }
        }
    }
    list = lord.query(".opPost:not(#postTemplate), .post:not(#postTemplate)");
    for (var i = 0; i < list.length; ++i) {
        if (lord.isInViewport(list[i]) && window.location.hash.replace("#", "") == list[i].id.replace("post", "")) {
            if (post || lord.text("currentThreadNumber")) {
                i = f(list, i);
                window.location.hash = list[i].id.replace("post", "");
            } else {
                window.location.hash = list[i].parentNode.id.replace("threadPosts", "");
            }
            return false;
        }
    }
    for (var i = 0; i < list.length; ++i) {
        if (lord.isInViewport(list[i])) {
            if (post || lord.text("currentThreadNumber")) {
                i = f(list, i);
                window.location.hash = list[i].id.replace("post", "");
            } else {
                window.location.hash = list[i].parentNode.id.replace("threadPosts", "");
            }
            return false;
        }
    }
};

lord.hotkey_previousThreadPost = function() {
    return lord.previousNextThreadPostCommon(false, false);
};

lord.hotkey_nextThreadPost = function() {
    return lord.previousNextThreadPostCommon(true, false);
};

lord.hotkey_previousPost = function() {
    return lord.previousNextThreadPostCommon(false, true);
};

lord.hotkey_nextPost = function() {
    return lord.previousNextThreadPostCommon(true, true);
};

lord.interceptHotkey = function(e) {
    if (e.target.tagName && lord.in(["TEXTAREA", "INPUT", "BUTTON"], e.target.tagName))
        return;
    var hotkeys = lord.getLocalObject("hotkeys", {});
    var key = e.key;
    if (e.metaKey)
        key = "Meta+" + key;
    if (e.altKey)
        key = "Alt+" + key;
    if (e.shiftKey)
        key = "Shift+" + key;
    if (e.ctrlKey)
        key = "Ctrl+" + key;
    var name = hotkeys.rev ? (hotkeys.rev[key] || lord.defaultHotkeys.rev[key]) : lord.defaultHotkeys.rev[key];
    if (!name || !lord["hotkey_" + name])
        return;
    if (lord["hotkey_" + name]() !== false)
        return;
    e.preventDefault();
    return false;
};

lord.initializeOnLoadBaseBoard = function() {
    document.body.onclick = lord.globalOnclick;
    document.body.onmouseover = lord.globalOnmouseover;
    document.body.onmouseout = lord.globalOnmouseout;
    if (lord.getLocalObject("hotkeysEnabled", true)) {
        document.body.addEventListener("keypress", lord.interceptHotkey, false);
        var hotkeys = lord.getLocalObject("hotkeys", {}).dir;
        var key = function(name) {
            if (!hotkeys)
                return lord.defaultHotkeys.dir[name];
            return hotkeys[name] || lord.defaultHotkeys.dir[name];
        };
        var btn = lord.queryOne(".leafButton.leafButtonPrevious");
        btn.title = btn.title + " (" + key("previousPageImage") + ")";
        btn = lord.queryOne(".leafButton.leafButtonNext");
        btn.title = btn.title + " (" + key("nextPageImage") + ")";
    }
    if (lord.getLocalObject("showTripcode", false)) {
        var postForm = lord.id("postForm");
        var sw = lord.nameOne("tripcode", postForm);
        sw.checked = true;
    }
    var fav = lord.getLocalObject("favoriteThreads", {});
    var currentBoardName = lord.text("currentBoardName");
    var list = lord.getLocalObject("hiddenPosts", {});
    var posts = lord.query(".post, .opPost");
    posts.forEach(function(post) {
        if (lord.getLocalObject("showYoutubeVideosTitles", true))
            lord.addYoutubeButton(post);
        lord.tryHidePost(post, list);
    });
    lord.query(".opPost").forEach(function(opPost) {
        var threadNumber = +opPost.id.replace("post", "");
        var btn = lord.nameOne("addToFavoritesButton", opPost);
        if (fav.hasOwnProperty(currentBoardName + "/" + threadNumber)) {
            var img = lord.queryOne("img", btn);
            img.title = lord.text("removeFromFavoritesText");
            img.src = img.src.replace("favorite.png", "favorite_active.png");
            btn.onclick = lord.removeThreadFromFavorites.bind(lord, currentBoardName, threadNumber);
        } else {
            btn.onclick = lord.addThreadToFavorites.bind(lord, currentBoardName, threadNumber);
        }
    });
    if (!!lord.getLocalObject("hideTripcodes", false)) {
        lord.query(".tripcode").forEach(function(span) {
            span.style.display = "none";
        });
    }
    if (!!lord.getLocalObject("hideUserNames", false)) {
        lord.query(".someName").forEach(function(span) {
            span.style.display = "none";
        });
    }
    var lastLang = lord.getLocalObject("lastCodeLang", "-");
    var sel = lord.queryOne(".postformMarkup > span > [name='langSelect']");
    lord.arr(sel.options).forEach(function(opt) {
        if (opt.value == lastLang)
            opt.selected = true;
    });
    lord.setPostformMarkupVisible(!lord.getLocalObject("hidePostformMarkup", false));
    if (!!lord.getLocalObject("strikeOutHiddenPostLinks", true))
        lord.strikeOutHiddenPostLinks();
    if (!lord.text("currentThreadNumber")) {
        var lastPostNumbers = lord.getLocalObject("lastPostNumbers", {});
        lastPostNumbers[lord.text("currentBoardName")] = +lord.text("lastPostNumber");
        lord.setLocalObject("lastPostNumbers", lastPostNumbers);
    }
    lord.initFiles();
};

window.addEventListener("load", function load() {
    window.removeEventListener("load", load, false);
    lord.initializeOnLoadBaseBoard();
}, false);
