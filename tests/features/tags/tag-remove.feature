Feature: Tag removal
  As someone using tasks and notes
  I can remove a tag
  In order to maintain their semantic

  Scenario: Removed tag disappear from the list
    Given I display the available pages
    When I remove a "tag" named "Optional"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-tasks      |
       | Contexts / Internet               | view-pim-tasks      |
       | Contexts / Online                 | view-pim-tasks      |
       | Inbox                             | mail-folder-inbox   |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Party                  | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Tags                              | folder              |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |
