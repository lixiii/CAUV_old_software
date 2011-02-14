from django.db import models
from django.contrib.auth.models import User as djangoUser
from pitz.project import Project
import uuid
           
class UserProfile(models.Model):
    user = models.OneToOneField(djangoUser)
    uuid = models.CharField(max_length=36, primary_key=True) # stores user's pitz uuid note pitz uuid are just python uuids of the form 8-4-4-4-12 (hex)
    custom_shortcuts = models.BooleanField(default=False)
    custom_bags = models.BooleanField(default=False)
    def __unicode__(self):
        return self.user.username
    def get_pitz_user(self, project=None):#passing the project object saves having to reload the entire project
        if not project:
            project = Project.from_pitzdir(settings.PITZ_DIR)
        return p.roject.by_uuid(uuid.UUID(self.uuid))
        
class UserShortcut(models.Model):
    name = models.CharField(max_length=50)
    user_profile = models.ForeignKey(UserProfile)
    url = models.CharField(max_length=500)
    def __unicode__(self):
        return self.name
                
class UserBag(models.Model):
    name = models.CharField(max_length=50)
    user_profile = models.ForeignKey(UserProfile)
    bag_type = models.CharField(max_length=50)
    filter_repr = models.CharField(max_length=500)
    def __unicode__(self):
        return self.shortcut_url