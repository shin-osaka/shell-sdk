/******************************************************************************
 * Spine Runtimes Software License v2.5
 *
 * Copyright (c) 2013-2016, Esoteric Software
 * All rights reserved.
 *
 * You are granted a perpetual, non-exclusive, non-sublicensable, and
 * non-transferable license to use, install, execute, and perform the Spine
 * Runtimes software and derivative works solely for personal or internal
 * use. Without the written permission of Esoteric Software (see Section 2 of
 * the Spine Software License Agreement), you may not (a) modify, translate,
 * adapt, or develop new applications using the Spine Runtimes or otherwise
 * create derivative works or improvements of the Spine Runtimes or (b) remove,
 * delete, alter, or obscure any trademarks or any copyright, trademark, patent,
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 *
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
 * USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef Spine_Skin_h
#define Spine_Skin_h

#include <spine/Vector.h>
#include <spine/SpineString.h>

namespace spine {
class Attachment;

class Skeleton;

class SP_API Skin : public SpineObject {
	friend class Skeleton;

public:
	class SP_API AttachmentMap : public SpineObject {
		friend class Skin;

	public:
		struct SP_API Entry {
			size_t _slotIndex;
			String _name;
			Attachment *_attachment;

			Entry(size_t slotIndex, const String &name, Attachment *attachment) :
					_slotIndex(slotIndex),
					_name(name),
					_attachment(attachment) {
			}
		};

		class SP_API Entries {
			friend class AttachmentMap;

		public:
			bool hasNext() {
				while(true) {
					if (_slotIndex >= _buckets.size()) return false;
					if (_bucketIndex >= _buckets[_slotIndex].size()) {
						_bucketIndex = 0;
						++_slotIndex;
						continue;
					};
					return true;
				}
			}

			Entry &next() {
				Entry &result = _buckets[_slotIndex][_bucketIndex];
				++_bucketIndex;
				return result;
			}

		protected:
			Entries(Vector< Vector<Entry> > &buckets) : _buckets(buckets), _slotIndex(0), _bucketIndex(0) {
			}

		private:
			Vector< Vector<Entry> > &_buckets;
			size_t _slotIndex;
			size_t _bucketIndex;
		};

		void put(size_t slotIndex, const String &attachmentName, Attachment *attachment);

		Attachment *get(size_t slotIndex, const String &attachmentName);

		void remove(size_t slotIndex, const String &attachmentName);

		Entries getEntries();

	protected:
		AttachmentMap();

	private:

		int findInBucket(Vector <Entry> &, const String &attachmentName);

		Vector <Vector<Entry> > _buckets;
	};

	explicit Skin(const String &name);

	~Skin();

	void addAttachment(size_t slotIndex, const String &name, Attachment *attachment);

	Attachment *getAttachment(size_t slotIndex, const String &name);

	void findNamesForSlot(size_t slotIndex, Vector <String> &names);

	void findAttachmentsForSlot(size_t slotIndex, Vector<Attachment *> &attachments);

	const String &getName();

	AttachmentMap::Entries getAttachments();

private:
	const String _name;
	AttachmentMap _attachments;

	void attachAll(Skeleton &skeleton, Skin &oldSkin);
};
}

#endif /* Spine_Skin_h */
