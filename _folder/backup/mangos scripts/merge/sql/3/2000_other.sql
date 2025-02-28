-- disable transmute for x100 because it can be duped
delete from skill_extra_item_template where requiredSpecialization=28672;